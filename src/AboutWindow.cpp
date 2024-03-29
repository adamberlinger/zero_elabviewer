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
#include "AboutWindow.h"

#include <QVBoxLayout>

AboutWindow::AboutWindow(){
    this->setLayout(layout = new QVBoxLayout());

    layout->addWidget(mainText = new QTextBrowser());
    mainText->setAlignment(Qt::AlignHCenter);
#ifdef APP_RC_VERSION
    layout->addWidget(new QLabel(QString("Version: %1.%2rc%3").arg(APP_MAJOR_VERSION).arg(APP_MINOR_VERSION).arg(APP_RC_VERSION)));
#else
    layout->addWidget(new QLabel(QString("Version: %1.%2").arg(APP_MAJOR_VERSION).arg(APP_MINOR_VERSION)));
#endif
    layout->addWidget(closeButton = new QPushButton("Close"));

    mainText->setSource(QUrl("qrc:/about.html"));
    mainText->setOpenExternalLinks(true);
    this->setMinimumSize(600,600);

    QObject::connect(closeButton, SIGNAL(pressed()), this, SLOT(hide()));
}

AboutWindow::~AboutWindow(){

}
