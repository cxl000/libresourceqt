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

#pragma once

#include <policy/resource-set.h>

#include "streamer.h"

class PlayerWidget : public QObject {
  Q_OBJECT

public:
  PlayerWidget(Streamer *streamer = 0);

  Streamer::State prevState;

  void play();
  void beginPlayback();
  void pause(bool releaseResources = true);
  void stop(bool releaseResources = true);
  void acquire();
  void release();

  bool policyAware();
  void setPolicyAware(bool aware);

  quint64 position();
  void setPosition(quint64);
  void seek(quint64);

  Streamer     *streamer;

  void open(const QString& filename);
  Streamer::State state();
  quint64 length();

  void error(const QString message);
  void eos(void);

private:

  ResourcePolicy::ResourceSet *resourceSet;
  ResourcePolicy::AudioResource *audioResource;

  struct data {
    quint64 pos;
    bool    policyAware;

    data() : pos(0), policyAware(true)  {}
  } d;

  void timerEvent(QTimerEvent *event);

private slots:
  void resourceAcquiredHandler(const QList<ResourcePolicy::ResourceType>& /*grantedOptionalResList*/);
  void resourceReleasedHandler();
  void resourceLostHandler();
  void resourceReleasedByManagerHandler();
  void resourcesDeniedHandler();

signals:
  void playerPositionChanged();
  void playing();
  void paused();
  void denied();

};
