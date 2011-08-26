/*************************************************************************
This file is part of libresourceqt

Copyright (C) 2011 Nokia Corporation.

This library is free software; you can redistribute
it and/or modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation
version 2.1 of the License.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301
USA.
*************************************************************************/

#include "playerwidget.h"

using namespace ResourcePolicy;

/**
  * Creates a resource set in the "player" application class, this application class
  * best suites our application description. The application class is used to determine the priority
  * order of the application.
  * We create one AudioResource object and add it to the set.  For that audio resource we
  * have to set the pid of the audio renderer, which in our case is the same as the pid of the application,
  * and stream tag, which we simply set to an all-allowing "*".
  *
  * Also, we connect to five signals:
  * - resourcesGranted, which is triggered whenever the resource we asked for is granted to us;
  * - lostResources, which is triggered whenever another application has been granted with our resource
  * and we are no longer able to use it;
  * - resourcesReleased, which is triggered whenever a call to release() is done and the resource
  * has been successfully released.
  * - resourcesReleasedByManager, which is triggered whenever resources has been released by the manager
  * (e.g. headset is released)
  * - resourcesresourcesDenied, which is triggered whenever resources has been denied by the manager
  * 
  * At last, we add a timer which fires every 100 milliseconds in order to update the current playback position.
  *
  * \see PlayerWidget::resourceAcquiredHandler
  * \see PlayerWidget::resourceLostHandler
  * \see PlayerWidget::resourceReleasedHandler
  * \see PlayerWidget::resourceReleasedByManagerHandler
  * \see PlayerWidget::resourcesDeniedHandler
  */
PlayerWidget::PlayerWidget(Streamer *streamer)
  : QObject() {

  this->streamer = streamer;
  this->streamer->start();

  connect(this->streamer, SIGNAL(eos()), this, SLOT(eos()));
  connect(this->streamer, SIGNAL(error(const QString)), this, SLOT(error(const QString)));

  qDebug("PlayerWidget::PlayerWidget");
  resourceSet = new ResourceSet("player", this);

  audioResource = new ResourcePolicy::AudioResource("player");
  audioResource->setProcessID(QCoreApplication::applicationPid());
  audioResource->setStreamTag("media.name", "*");
  resourceSet->addResourceObject(audioResource);

  connect(resourceSet, SIGNAL(resourcesGranted(const QList<ResourcePolicy::ResourceType>&)),
          this,        SLOT(resourceAcquiredHandler(const QList<ResourcePolicy::ResourceType>&)));
  connect(resourceSet, SIGNAL(lostResources()),     this, SLOT(resourceLostHandler()));
  connect(resourceSet, SIGNAL(resourcesReleased()), this, SLOT(resourceReleasedHandler()));
  connect(resourceSet, SIGNAL(resourcesReleasedByManager()), this, SLOT(resourceReleasedByManagerHandler()));
  connect(resourceSet, SIGNAL(resourcesDenied()), this, SLOT(resourcesDeniedHandler()));


  // playback timer
  startTimer(100);
}

void PlayerWidget::error(const QString message)
{
  qCritical() << QString("Streamer error: %1").arg(message);
  pause();
}

void PlayerWidget::eos(void)
{
  qDebug() << QString("end of stream");
  stop();
}

/**
  * Calls acquire() on the resource set.  Shortly after we should receive a notification
  * when resources are granted, which means we can begin using audio output.
  *
  * \see PlayerWidget::resourceAcquiredHandler
  *
  */
void PlayerWidget::acquire() {
  resourceSet->acquire();
}

/**
  * Calls release() on the resource set.  Called whenever we are not using the resource.
  * We can acquire the same resource set again if required.  We release resource every time
  * a pause button is pressed or audio playback is finished, so that other applications can
  * make use of it.
  * Call to release() yields resourceReleased() signal shortly after to inform us that resource
  * has indeed been released.
  *
  * \see PlayerWidget::resourceReleasedHandler
  *
  */
void PlayerWidget::release() {
  resourceSet->release();
}

/**
  * If we are in resource policy aware mode, asks to acquire the resource,
  * otherwise begins playback immediately.
  */
void PlayerWidget::play() {
  qDebug("PlayerWidget::play()");
  if (policyAware())
    acquire();
  else
    beginPlayback();
}

/**
  * Calls parent play() method to begin playback and emits the playing() signal.
  * This function is used in place of play(), which only prepares the playback
  * prior to actually starting it.
  *
  */
void PlayerWidget::beginPlayback() {
  streamer->play();
  seek(d.pos) ;
  emit playing();
}

/**
  * Releases the audio resource on pause so
  * that another application can start using it immediately.  Emits paused() signal.
  * An optional parameter releaseResources is added to opt out of releasing the
  * resource in PlayerWidget::resourceLostHandler() handler.
  */
void PlayerWidget::pause(bool releaseResources) {
  streamer->pause();
  if (releaseResources && policyAware())  release();
  emit paused();
}

/**
  * If we are in resource policy aware mode, asks to release the resource,
  * otherwise just stop playback.
  */
void PlayerWidget::stop(bool releaseResources) {
  streamer->stop();
  if (releaseResources && policyAware())  release();
  setPosition(0);
  emit paused();
}

/**
  * The most interesting kind of event in resource policy.  This event
  * arrives to notify us that the resources we asked for are available for us to use.
  * Note that not all resources of the resource set might be granted, but we have
  * only one resource in total (audio).  If the granted resources list is not empty,
  * we begin the playback.
  *
  * \see signal ResourcePolicy::ResourceSet::resourcesGranted().
  */
void PlayerWidget::resourceAcquiredHandler(const QList<ResourcePolicy::ResourceType>& /*grantedOptionalResList*/) {
  qDebug("PlayerWidget::resourceAcquiredHandler()");
  beginPlayback();
}

/**
  * This notification arrives to acknowledge that we no longer own the resources from
  * our resource set.  The handler is a no-op in our case.
  *
  * \see signal ResourcePolicy::ResourceSet::resourcesReleased().
  */
void PlayerWidget::resourceReleasedHandler() {
  qDebug("PlayerWidget::resourceReleasedHandler()");
}

/**
  * This signals that the resources has been released by the manager, for instance
  * when the headset is released or after a call.
  *
  * \see signal ResourcePolicy::ResourceSet::resourceReleasedByManager().
  */
void PlayerWidget::resourceReleasedByManagerHandler() {
  qDebug("PlayerWidget::resourceReleasedByManagerHandler()");
  if (state() == Streamer::PlayingState) {
  pause(false);
  }
}

/**
  * This signals that the resources has been denied by the manager
  *
  * \see signal ResourcePolicy::ResourceSet::resourcesDenied().
  */
void PlayerWidget::resourcesDeniedHandler() {
  emit denied();
}

/**
  * Handles the event of a resource being taken by another application.
  * Our application is forced to stop using audio resource by muting the sound output.
  * We will pause the audio, but will not release the resource yet,
  * so that we receive the acquire notification as soon as the audio resource
  * is available again.
  *
  * \see signal ResourcePolicy::ResourceSet::lostResources().
  */
void PlayerWidget::resourceLostHandler() {
  qDebug("PlayerWidget::resourceLostHandler()");
  if (state() == Streamer::PlayingState) {
    pause(false);
  }
}

/**
  * Timer event is used to update current playback position.  Our own instance variable, d.pos, is used.
  * It also handles the Stopped state, in which case paused() signal will be issued.
  * Every time we emit playerPositionChanged() for the subscribed widget to do its own
  * processing as well.
  *
  */
void PlayerWidget::timerEvent(QTimerEvent */*event*/) {    
  if ( length() < d.pos && state() == Streamer::PlayingState) {
        stop();
        d.pos = 0;
        emit playerPositionChanged();
        return;
  }

  qDebug() << "PlayerWidget::timerEvent state=" << (int)state();

  if (state() == Streamer::PlayingState) {
    d.pos += 100;
    emit playerPositionChanged();
  }

  if (state() == Streamer::StoppedState && prevState != state()) {
    pause();
    prevState = state();
  }

}

/**
  * Getter for policyAware property, which can be set to true to make the demo behave
  * like policy-aware and to false for otherwise.
  */
bool PlayerWidget::policyAware() {
  return d.policyAware;
}

/**
  * Setter for policyAware property, which can be set to true to make the demo behave
  * like policy-aware and to false for otherwise.
  */
void PlayerWidget::setPolicyAware(bool aware) {
  d.policyAware = aware;

  if (policyAware()) {
    if (state() == Streamer::PlayingState)  acquire();
  } else {
    release();
  }
}

/**
  * Getter for position property, our own current playback position counter.
  */
quint64 PlayerWidget::position() {
  return d.pos;
}

/**
  * Setter for position property, our own current playback position counter.
  */
void PlayerWidget::setPosition(quint64 pos) {
  d.pos = pos;
  emit playerPositionChanged();
}

/**
  * Updates our own playback position counter.
  */
void PlayerWidget::seek(quint64 pos) {
  setPosition(pos);
  streamer->setPosition(pos);
}

/**
  * Setter for current audio file.
  */
void PlayerWidget::open(const QString& filename) {
  streamer->setLocation(filename);
}

/**
  * Getter for streamer state.   
  * Can return PlayingState, PausedState or StoppedState
  */
Streamer::State PlayerWidget::state() {
  return streamer->state();
}

/**
  * Getter for audio file length.   
  */
quint64 PlayerWidget::length() {
  return streamer->duration();
}

