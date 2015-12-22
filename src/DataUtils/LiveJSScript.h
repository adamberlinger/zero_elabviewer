/*
 * Copyright 2016 Adam Berlinger
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>
 */
#ifndef _LIVE_JS_SCRIPT_H_
#define _LIVE_JS_SCRIPT_H_

#include <QtCore>
#include <QtGlobal>
#include <QJSEngine>

class LiveJSScript : public QObject {
    Q_OBJECT
protected:
    QJSEngine* engine;
    QFileSystemWatcher fileWatcher;
    QJSValue jsScript;
    QString filePath;
    bool isWorking;

protected slots:
    void loadScript();
public:
    LiveJSScript(QJSEngine* engine, QString filePath);
    QJSValue call(QJSValueList args);
    virtual ~LiveJSScript();
};

#endif
