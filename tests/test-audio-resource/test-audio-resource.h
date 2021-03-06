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

#ifndef TESTAUDIORESOURCE_H
#define TESTAUDIORESOURCE_H

#include <QtTest/QTest>
#include <QObject>
#include <policy/audio-resource.h>

using namespace ResourcePolicy;

class TestAudioResource: public QObject
{
    Q_OBJECT
private:
    AudioResource *audioResource;

public:
    TestAudioResource();
    ~TestAudioResource();

private slots:

    void testConstruct1();
    void testConstruct2();
    void testCopyConstruct();
    void testSetAudioGroup();
    void testSetProcessId();
    void testSetStreamTag();
};

#endif // TESTAUDIORESOURCE_H
