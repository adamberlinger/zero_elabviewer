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
#include "HelpWindow.h"

#include <QVBoxLayout>

HelpWindow::HelpWindow(QString help_path){
    this->setLayout(layout = new QVBoxLayout());

    layout->addWidget(mainText = new QTextBrowser());
    mainText->setAlignment(Qt::AlignHCenter);
    layout->addWidget(closeButton = new QPushButton("Close"));

    basePath = help_path;
    mainText->setSource(QUrl(basePath));
    mainText->setOpenExternalLinks(true);

    QObject::connect(closeButton, SIGNAL(pressed()), this, SLOT(hide()));

    this->setMinimumSize(800,600);
}

void HelpWindow::setAnchor(QString anchor){
    mainText->scrollToAnchor(anchor);
}

HelpWindow::~HelpWindow(){

}
