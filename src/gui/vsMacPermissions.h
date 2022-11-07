//*****************************************************************
/*
  JackTrip: A System for High-Quality Audio Network Performance
  over the Internet

  Copyright (c) 2008-2021 Juan-Pablo Caceres, Chris Chafe.
  SoundWIRE group at CCRMA, Stanford University.

  Permission is hereby granted, free of charge, to any person
  obtaining a copy of this software and associated documentation
  files (the "Software"), to deal in the Software without
  restriction, including without limitation the rights to use,
  copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the
  Software is furnished to do so, subject to the following
  conditions:

  The above copyright notice and this permission notice shall be
  included in all copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
  OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
  NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
  HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
  WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
  FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
  OTHER DEALINGS IN THE SOFTWARE.
*/
//*****************************************************************
/**
 * \file vsMacPermissions.h
 * \author Matt Horton
 * \date Oct 2022
 */

#ifndef __VSMACPERMISSIONS_H__
#define __VSMACPERMISSIONS_H__

#include <objc/objc.h>

#include <QDebug>
#include <QObject>
#include <QString>

class VsMacPermissions : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString micPermission READ micPermission NOTIFY micPermissionUpdated)

   public:
    VsMacPermissions(){};
    ~VsMacPermissions(){};

    QString micPermission();
    Q_INVOKABLE void getMicPermission();
    void setMicPermission(QString status);
    Q_INVOKABLE void openSystemPrivacy();

   signals:
    void micPermissionUpdated();

   private:
    QString m_micPermission = "unknown";
};

#endif  // __VSMACPERMISSIONS_H__
