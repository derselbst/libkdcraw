/* ============================================================
 * Authors: Gilles Caulier <caulier dot gilles at kdemail dot net> 
 *          Marcel Wiesweg <marcel.wiesweg@gmx.de>
 * Date   : 2006-12-09
 * Description : dcraw program interface
 *
 * Copyright 2006-2007 by Gilles Caulier and Marcel Wiesweg
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#ifndef DCRAW_IFACE_H
#define DCRAW_IFACE_H

// Qt Includes.

#include <qstring.h>
#include <qobject.h>
#include <qimage.h>

// Local includes.

#include "libkdcraw_export.h"
#include "rawdecodingsettings.h"
#include "dcrawinfocontainer.h"

class QCustomEvent;

class KProcess;

namespace KDcrawIface
{

class DcrawIfacePriv;

class LIBKDCRAW_EXPORT DcrawIface : public QObject
{
    Q_OBJECT

public:

    DcrawIface();
    ~DcrawIface();

/** Fast and non cancelable methods witch do not require a class instance to run.*/
public:  

    /** Get the embedded preview image from RAW pictures.
    */
    static bool loadDcrawPreview(QImage& image, const QString& path);

    /** Get the camera settings witch have taken RAW file. Look into dcrawinfocontainer.h 
        for more details.
    */ 
    static bool rawFileIdentify(DcrawInfoContainer& identify, const QString& path);

/** Cancelable methods to extract RAW data witch require a class instance to run. 
    RAW pictures decoding can take a while.*/
public: 

    /** To cancel 'decodeHalfRAWImage' and 'decodeRAWImage' methods running 
        in a separate thread.
    */
    void cancel();

    /** Extract a small size of decode RAW data using 'rawDecodingSettings' settings.
    */
    bool decodeHalfRAWImage(const QString& filePath, RawDecodingSettings rawDecodingSettings, QByteArray &imageData);

    /** Extract a full size of RAW data using 'rawDecodingSettings' settings.
    */
    bool decodeRAWImage(const QString& filePath, RawDecodingSettings rawDecodingSettings, QByteArray &imageData);

private:

    bool loadFromDcraw(const QString& filePath, QByteArray &imageData);
    void startProcess();

    virtual void customEvent(QCustomEvent *);

private slots:

    void slotProcessExited(KProcess *);
    void slotReceivedStdout(KProcess *, char *, int);
    void slotReceivedStderr(KProcess *, char *, int);
    void slotContinueQuery();

private:

    DcrawIfacePriv *d;
};

}  // namespace KDcrawIface

#endif /* DCRAW_IFACE_H */
