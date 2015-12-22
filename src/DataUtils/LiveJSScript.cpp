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
#include "LiveJSScript.h"
LiveJSScript::LiveJSScript(QJSEngine* engine, QString filePath){
    this->engine = engine;
    this->filePath = filePath;

    this->loadScript();

    fileWatcher.addPath(filePath);

    QObject::connect(&fileWatcher, SIGNAL(fileChanged(const QString)), this, SLOT(loadScript()));
}

void LiveJSScript::loadScript(){
    qDebug() << "JS Script refreshed";

    QFile scriptFile(filePath);
    if (!scriptFile.open(QIODevice::ReadOnly)){
        qDebug() << "File not opened";
        isWorking = false;
    }
    QTextStream stream(&scriptFile);
    QString contents = stream.readAll();
    scriptFile.close();

    jsScript = engine->evaluate(contents, filePath);

    if(jsScript.isError()){
        qDebug() << jsScript.toString();
        isWorking = false;
    }
    else {
        isWorking = true;
    }
}

QJSValue LiveJSScript::call(QJSValueList args){
    if(isWorking){
        return jsScript.call(args);
    }
    else {
        return QJSValue();
    }
}
LiveJSScript::~LiveJSScript(){

}
