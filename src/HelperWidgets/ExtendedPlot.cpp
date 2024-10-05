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
#include "ExtendedPlot.h"
#include <iostream>
#include <cmath>

const QColor ExtendedPlot::graph_colors[GRAPH_COLORS_SIZE] = {
    Qt::blue, QColor(0,128,0), QColor(128,0,128),
    QColor(128,0,0)
};

const double ExtendedPlot::Cursor::big_double = 10000.0;

ExtendedPlot::CursorSpace::CursorSpace(ExtendedPlot* plot,Cursor* prev, Cursor* next):prev(prev),next(next){
    double delta =  std::fabs(next->position - prev->position);

    text = new QCPItemText(plot);
    bracket = new QCPItemBracket(plot);

    if(prev->type == Cursor::X_AXIS){
        double frequency = plot->frequencyMultiplier / delta;

        bracket->left->setTypeX(QCPItemPosition::ptPlotCoords);
        bracket->left->setTypeY(QCPItemPosition::ptAbsolute);
        bracket->left->setCoords(prev->position, 25.0);
        bracket->left->setParentAnchorX(prev->start);

        bracket->right->setTypeX(QCPItemPosition::ptPlotCoords);
        bracket->right->setTypeY(QCPItemPosition::ptAbsolute);
        bracket->right->setCoords(next->position, 25.0);
        bracket->right->setParentAnchorX(next->start);

        text->position->setTypeY(QCPItemPosition::ptAbsolute);
        text->position->setCoords(0, 0.0);
        text->position->setParentAnchorX(bracket->center);
        text->setPositionAlignment(Qt::AlignHCenter | Qt::AlignTop);
    }
    else {
        bracket->left->setTypeX(QCPItemPosition::ptAbsolute);
        bracket->left->setTypeY(QCPItemPosition::ptPlotCoords);
        bracket->left->setCoords(35.0,prev->position);
        bracket->left->setParentAnchorY(prev->start);

        bracket->right->setTypeX(QCPItemPosition::ptAbsolute);
        bracket->right->setTypeY(QCPItemPosition::ptPlotCoords);
        bracket->right->setCoords(35.0, next->position);
        bracket->right->setParentAnchorY(next->start);

        text->position->setTypeX(QCPItemPosition::ptAbsolute);
        text->position->setCoords(0, 0.0);
        text->position->setParentAnchorY(bracket->center);
        text->setPositionAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    }

    text->setClipToAxisRect(false);
    text->setBrush(Qt::darkYellow);
    text->setPen(QPen(Qt::darkYellow));
    text->setColor(Qt::white);
    text->setPadding(QMargins(5,2,5,2));
    text->setLayer(plot->layer("legend"));

    bracket->setClipToAxisRect(false);
    bracket->setPen(QPen(Qt::darkYellow));
    bracket->setStyle(QCPItemBracket::bsSquare);
    bracket->setLength(0.0);

    this->update(plot);
}

void ExtendedPlot::CursorSpace::update(ExtendedPlot* plot){
    double delta =  std::fabs(next->position - prev->position);
    if(prev->type == Cursor::X_AXIS){
        double scaledDelta;
        const char* unitPrefix = DataConverter::getUnitPrefix(delta / plot->frequencyMultiplier, &scaledDelta);
        if(delta > 0.000001){
            double frequency = plot->frequencyMultiplier / delta;
            double scaledFrequency;
            const char* unitPrefixFrequency = DataConverter::getUnitPrefix(frequency, &scaledFrequency);
            QString label = QString(plot->xCursorSpaceLabel);
            label.replace("%{x}", QString::number(delta,'f',3));
            label.replace("%{x'}", QString::number(scaledDelta,'f',3));
            label.replace("%{f}", QString::number(frequency,'f',3));
            label.replace("%{f'}", QString::number(scaledFrequency,'f',3));
            label.replace("%{u}", unitPrefix);
            label.replace("%{uf}", unitPrefixFrequency);
            text->setText(label);
        }
        else {
            QString label = QString(plot->xCursorSpaceLabel);
            label.replace("%{x}", QString::number(delta,'f',3));
            label.replace("%{x'}", QString::number(scaledDelta,'f',3));
            label.replace("%{f}", "---");
            label.replace("%{f'}", "---");
            label.replace("%{uf}", "");
            label.replace("%{u}", unitPrefix);
            text->setText(label);
        }
    }
    else {
        double scaledDelta;
        const char* unitPrefix = DataConverter::getUnitPrefix(delta, &scaledDelta);
        QString label = QString(plot->yCursorSpaceLabel);
        label.replace("%{y}", QString::number(delta,'f',3));
        label.replace("%{y'}", QString::number(scaledDelta,'f',3));
        label.replace("%{u}", unitPrefix);
        text->setText(label);
    }
}

void ExtendedPlot::CursorSpace::setVisible(bool on){
    bracket->setVisible(on);
    text->setVisible(on);
}

void ExtendedPlot::CursorSpace::remove(ExtendedPlot* plot){
    plot->removeItem(text);
    plot->removeItem(bracket);
}


ExtendedPlot::CursorSpace::~CursorSpace(){

}

ExtendedPlot::Cursor::Cursor(double position, Cursor::Type type, ExtendedPlot* plot):QCPItemLine(plot),type(type),position(position){
    this->text = new QCPItemText(plot);

    /* Finite number required for itemAt */
    if(type == X_AXIS){
        this->start->setCoords( position, -big_double);
        this->end->setCoords( position, big_double);

        text->setPositionAlignment(Qt::AlignHCenter | Qt::AlignTop);
        text->position->setTypeX(QCPItemPosition::ptPlotCoords);
        text->position->setTypeY(QCPItemPosition::ptAbsolute);
        text->position->setCoords( position, 0.0);
    }
    else {
        this->start->setCoords(-big_double, position);
        this->end->setCoords(big_double, position);

        text->setPositionAlignment(Qt::AlignLeft | Qt::AlignVCenter);
        text->position->setTypeX(QCPItemPosition::ptAbsolute);
        text->position->setTypeY(QCPItemPosition::ptPlotCoords);
        text->position->setCoords(0.0, position);
    }
    text->setLayer(plot->layer("legend"));

    this->setPen(QPen(Qt::darkYellow));
    this->setSelectable(true);

    text->setBrush(Qt::darkYellow);
    text->setPen(QPen(Qt::darkYellow));
    text->setColor(Qt::white);
    text->setClipToAxisRect(false);
    text->setPadding(QMargins(5,2,5,2));

    spacePrev = spaceNext = NULL;

    this->move(position,plot);
}

void ExtendedPlot::Cursor::move(double position,ExtendedPlot* plot){
    this->position = position;
    if(type == X_AXIS){
        double scaledPosition;
        const char* unitPrefix = DataConverter::getUnitPrefix(position / plot->frequencyMultiplier, &scaledPosition);
        this->start->setCoords(position,-big_double);
        this->end->setCoords(position,big_double);
        QString label = QString(plot->xCursorLabel);
        label.replace("%{x}", QString::number(position,'f',3));
        label.replace("%{x'}", QString::number(scaledPosition,'f',3));
        label.replace("%{u}", unitPrefix);
        text->setText(label);
        text->position->setCoords(position, 0.0);
    }
    else {
        double scaledPosition;
        const char* unitPrefix = DataConverter::getUnitPrefix(position, &scaledPosition);
        this->start->setCoords(-big_double,position);
        this->end->setCoords(big_double,position);
        QString label = QString(plot->yCursorLabel);
        label.replace("%{y}", QString::number(position,'f',3));
        label.replace("%{y'}", QString::number(scaledPosition,'f',3));
        label.replace("%{u}", unitPrefix);
        text->setText(label);
        text->position->setCoords(0.0, position);
    }
    if(spacePrev){
        spacePrev->update(plot);
    }
    if(spaceNext){
        spaceNext->update(plot);
    }
}

void ExtendedPlot::Cursor::setVisible(bool on){
    QCPItemLine::setVisible(on);
    text->setVisible(on);
}

void ExtendedPlot::Cursor::remove(ExtendedPlot* plot){
    plot->removeItem(this->text);
}

ExtendedPlot::Cursor::~Cursor(){

}

ExtendedPlot::ExtendedPlot(QWidget* parent):QCustomPlot(parent),
    xCursorLabel("%{x'} %{u}s"),yCursorLabel("%{y} V"),
    xCursorSpaceLabel("f = %{f'} %{uf}Hz\nΔt = %{x'} %{u}s"),yCursorSpaceLabel("ΔV = %{y'} %{u}V"){

    userZoomed = false;
    lazyZoomSetting = false;
    dragAction = DRAG_NONE;
    lastCursorX = NULL;
    lastCursorY = NULL;
    cursorsVisible = true;
    frequencyMultiplier = 1000.0;

    QObject::connect (this->xAxis, SIGNAL(rangeChanged(const QCPRange&)), this,
        SLOT(userZoomChanged(const QCPRange&)));
    QObject::connect (this->yAxis, SIGNAL(rangeChanged(const QCPRange&)), this,
        SLOT(userZoomChanged(const QCPRange&)));


    QObject::connect(this, SIGNAL(mousePress(QMouseEvent*)),
        this, SLOT(selectZoomStart(QMouseEvent*)));
    QObject::connect(this, SIGNAL(mouseMove(QMouseEvent*)),
        this, SLOT(selectZoomMove(QMouseEvent*)));
    QObject::connect(this, SIGNAL(mouseRelease(QMouseEvent*)),
        this, SLOT(selectZoomStop(QMouseEvent*)));

    selectRubberBand = new QRubberBand(QRubberBand::Rectangle,this);
    contextMenu = new QMenu(tr("Context menu"),this);
    resetZoomAction = new QAction("Reset zoom", this);
    showPointsAction = new QAction("Show points", this);
    storeDataAction = new QAction("Store data (please disable offset)", this);
    addXCursorAction = new QAction(tr("Add X cursor"),this);
    addYCursorAction = new QAction(tr("Add Y cursor"),this);
    addXYCursorAction = new QAction(tr("Add XY cursors"),this);
    showHideCursorsAction = new QAction(tr("Hide cursors"),this);
    removeAllCursorAction = new QAction(tr("Remove all cursors"), this);
    showPointsAction->setCheckable(true);
    contextMenu->addAction(resetZoomAction);
    contextMenu->addAction(showPointsAction);
    contextMenu->addAction(storeDataAction);
    contextMenu->addAction(addXCursorAction);
    contextMenu->addAction(addYCursorAction);
    contextMenu->addAction(addXYCursorAction);
    contextMenu->addAction(showHideCursorsAction);
    contextMenu->addAction(removeAllCursorAction);

    QObject::connect (resetZoomAction, SIGNAL(triggered()), this, SLOT(resetZoomAndReplot()));
    QObject::connect (showPointsAction, SIGNAL(triggered(bool)), this, SLOT(showPointsCallback(bool)));
    QObject::connect (storeDataAction, SIGNAL(triggered()), this, SLOT(saveData()));
    QObject::connect (addXCursorAction, SIGNAL(triggered()), this, SLOT(addXCursor()));
    QObject::connect (addYCursorAction, SIGNAL(triggered()), this, SLOT(addYCursor()));
    QObject::connect (addXYCursorAction, SIGNAL(triggered()), this, SLOT(addXCursor()));
    QObject::connect (addXYCursorAction, SIGNAL(triggered()), this, SLOT(addYCursor()));
    QObject::connect (showHideCursorsAction, SIGNAL(triggered()), this, SLOT(showHideCursors()));
    QObject::connect (removeAllCursorAction, SIGNAL(triggered()), this, SLOT(removeAllCursors()));
}

void ExtendedPlot::setFrequencyMultiplier(double value){
    if(value < 0.0){
        frequencyMultiplier = 0.0;
    }
    else {
        frequencyMultiplier = value;
    }
}

void ExtendedPlot::addXCursor(){
    if(!cursorsVisible){
        showHideCursors();
    }

    double position = xAxis->pixelToCoord(contextMenuPosition.x());
    Cursor* c = new Cursor(position,Cursor::X_AXIS,this);
    cursors.append(c);

    if(lastCursorX != NULL){
        CursorSpace *space = new CursorSpace(this,lastCursorX,c);
        cursorSpaces.append(space);
        lastCursorX->spaceNext = space;
        c->spacePrev = space;
        lastCursorX = NULL;
    }
    else {
        lastCursorX = c;
    }

    this->replot();
}

void ExtendedPlot::addYCursor(){
    if(!cursorsVisible){
        showHideCursors();
    }

    double position = yAxis->pixelToCoord(contextMenuPosition.y());
    Cursor* c = new Cursor(position,Cursor::Y_AXIS,this);
    cursors.append(c);

    if(lastCursorY != NULL){
        CursorSpace *space = new CursorSpace(this,lastCursorY,c);
        cursorSpaces.append(space);
        lastCursorY->spaceNext = space;
        c->spacePrev = space;
        lastCursorY = NULL;
    }
    else {
        lastCursorY = c;
    }

    this->replot();
}

void ExtendedPlot::showHideCursors(){
    if(cursorsVisible){
        showHideCursorsAction->setText("Show cursors");
    }
    else {
        showHideCursorsAction->setText("Hide cursors");
    }
    cursorsVisible = !cursorsVisible;
    foreach(CursorSpace* space,cursorSpaces){
        space->setVisible(cursorsVisible);
    }
    foreach(Cursor *c, cursors){
        c->setVisible(cursorsVisible);
    }
    this->replot();
}


void ExtendedPlot::updateCursorLabels(){
    this->updateCursorLabels(NULL,NULL);
}


void  ExtendedPlot::updateCursorLabels(conv_double_callback_t xAxisConvert, conv_double_callback_t yAxisConvert){
    foreach(Cursor *c, cursors){
        if(c->type == Cursor::X_AXIS){
            double p = c->position;
            if(xAxisConvert) p = xAxisConvert(p);
            c->move(p,this);
        }
        else {
            double p = c->position;
            if(yAxisConvert) p = yAxisConvert(p);
            c->move(p,this);
        }
    }
    this->replot();
}

void ExtendedPlot::removeAllCursors(){
    lastCursorX = NULL;
    lastCursorY = NULL;
    foreach(CursorSpace* space,cursorSpaces){
        space->remove(this);
        delete space;
    }
    cursorSpaces.clear();
    foreach(Cursor *c, cursors){
        c->remove(this);
        /* This deletes the Cursor */
        this->removeItem(c);
    }
    cursors.clear();
    this->replot();
}

void ExtendedPlot::saveData(){
    QString filePath = QFileDialog::getSaveFileName(this,
        "Save File", QString(), "CSV (*.csv);;MATLAB/Octave (*.mat)");

    double t_mult = 1 / frequencyMultiplier;
    if(!filePath.isNull()){

        QFile file(filePath);
        file.open(QIODevice::WriteOnly);

        QTextStream textStream(&file);
        textStream << "Time (s)";
        QVector<QCPGraphDataContainer::const_iterator> it(this->graphCount());
        QVector<QCPGraphDataContainer::const_iterator> end(this->graphCount());
        for(int i = 0;i < this->graphCount();++i){
            QSharedPointer<QCPGraphDataContainer> data = this->graph(i)->data();
            it[i]  = data->constBegin();
            end[i] = data->constEnd();
            textStream << ",CH" << (i+1);
        }
        textStream << "\n";

        while(it[0] < end[0]){
            textStream << (it[0]->key * t_mult) << "," << it[0]->value;
            for(int i = 1; i < this->graphCount();++i){
                textStream << ",";
                if(it[i] < end[i]){
                    textStream << it[i]->value;
                    ++it[i];
                }
            }
            textStream << "\n";
            ++it[0];
        }
        file.close();
    }
}

void ExtendedPlot::setShowPoints(bool show){
    showPointsAction->setChecked(show);
    if(show){
        for(int i = 0;i < this->graphCount();++i){
            this->graph(i)->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssDisc, 5));
        }
    }
    else {
        for(int i = 0;i < this->graphCount();++i){
            this->graph(i)->setScatterStyle(QCPScatterStyle::ssNone);
        }
    }
}

void ExtendedPlot::setDataSet(DataSet* data){
    int newGraphCount = 0;
    for(int i = 0;i < data->length();++i){
        newGraphCount += (data->getData(i)->getType() == DataStream::DATA_BITS)?8:1;
    }

    while(this->graphCount() < newGraphCount){
        QCPGraph *newGraph = this->addGraph();
        if(showPointsAction->isChecked()){
            newGraph->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssDisc, 5));
        }
    }

    for(int i = this->graphCount(); newGraphCount < i;--i){
        this->removeGraph(i-1);
    }

    for(int i = 0,g_index=0;i < data->length();++i){
        if(data->getData(i)->getType() == DataStream::DATA_ANALOG){
            this->graph(g_index)->setData(*data->getXAxis(), *data->getData(i)->getDouble());
            this->graph(g_index)->setLineStyle(QCPGraph::lsLine);
            this->graph(g_index)->setPen(QPen(graph_colors[i % GRAPH_COLORS_SIZE]));
            g_index++;
        }
        else {
            int l = data->getData(i)->length();
            QVector<double> tmpData(l);
            QVector<uint8_t> *rawData = data->getData(i)->getBits();
            for(int x = 0;x < 8;++x){
                uint8_t mask = (1 << x);
                for(int t = 0;t < l;++t){
                    tmpData[t] = ((*rawData)[t] & mask)?0.25:0.0;
                    tmpData[t] += x * 0.5 + data->getData(i)->getOffset();
                }
                this->graph(g_index)->setData(*data->getXAxis(),tmpData);
                this->graph(g_index)->setLineStyle(QCPGraph::lsStepCenter);
                this->graph(g_index)->setPen(QPen(Qt::black));
                g_index++;
            }
        }
    }
}

void ExtendedPlot::showPointsCallback(bool show){
    QCPScatterStyle scatterSyle = (show)?QCPScatterStyle(QCPScatterStyle::ssDisc, 5):
                QCPScatterStyle::ssNone;

    for(int i = 0;i < this->graphCount();++i){
        this->graph(i)->setScatterStyle(scatterSyle);
    }
    this->replot();
}

void ExtendedPlot::resetZoomAndReplot(){
    this->resetZoom();
    this->replot();
}

void ExtendedPlot::lazyZoomX(double start, double end){
    lazyRangeX[0] = start;
    lazyRangeX[1] = end;

    if(userZoomed)
        return;

    lazyZoomSetting = true;
    this->xAxis->setRange(start,end);
    lazyZoomSetting = false;
}

void ExtendedPlot::lazyZoomY(double start, double end){
    lazyRangeY[0] = start;
    lazyRangeY[1] = end;

    if(userZoomed)
        return;

    lazyZoomSetting = true;
    this->yAxis->setRange(start,end);
    lazyZoomSetting = false;
}

void ExtendedPlot::userZoomChanged(const QCPRange &newRange){
    if(!lazyZoomSetting){
        userZoomed = true;
    }
}

void ExtendedPlot::selectZoomStart(QMouseEvent *event){
    if(dragAction == DRAG_NONE){
        if(event->button() == Qt::RightButton)
        {
            selectStartPoint = event->pos();
            selectRubberBand->setGeometry(QRect(event->pos(), QSize()));
            selectRubberBand->show();
            dragAction = DRAG_ZOOM;
        }
        else if(event->button() == Qt::LeftButton)
        {
            /* Select nearest X / Y cursors see itemAt from qcustomplot */
            Cursor *cursor;
            double resultDistanceX = mSelectionTolerance;
            double resultDistanceY = mSelectionTolerance;
            selectedCursorX = NULL;
            selectedCursorY = NULL;

            foreach (QCPAbstractItem *item, mItems)
            {
                if (!item->selectable())
                    continue;
                double currentDistance = item->selectTest(event->pos(), false);
                if(currentDistance >= 0.0){
                    cursor = dynamic_cast<Cursor*>(item);
                    if(cursor){
                        if(cursor->type == Cursor::X_AXIS){
                            if (currentDistance >= 0 && currentDistance < resultDistanceX)
                            {
                              selectedCursorX = cursor;
                              resultDistanceX = currentDistance;
                            }
                        }
                        else {
                            if (currentDistance >= 0 && currentDistance < resultDistanceY)
                            {
                              selectedCursorY = cursor;
                              resultDistanceY = currentDistance;
                            }
                        }
                    }
                }
            }

            if(selectedCursorX || selectedCursorY){
                this->axisRect(0)->setRangeDrag({});
                dragAction = DRAG_CURSOR;
            }
        }
    }
}

void ExtendedPlot::selectZoomMove(QMouseEvent* event){
    if(dragAction == DRAG_ZOOM){
        selectRubberBand->setGeometry(QRect(selectStartPoint, event->pos()).normalized());
    }
    else if(dragAction == DRAG_CURSOR){
        double position;
        if(selectedCursorX){
            position = xAxis->pixelToCoord(event->pos().x());
            selectedCursorX->move(position,this);
        }
        if(selectedCursorY){
            position = yAxis->pixelToCoord(event->pos().y());
            selectedCursorY->move(position,this);
        }
        this->replot();
    }
}

void ExtendedPlot::selectZoomStop(QMouseEvent *event){
    if(dragAction == DRAG_ZOOM){
        QRect zoomRect = selectRubberBand->geometry();
        int xp1, yp1, xp2, yp2;
        zoomRect.getCoords(&xp1, &yp1, &xp2, &yp2);
        if(((xp2 - xp1) <= 2) && ((yp2 - yp1) <= 2)){
            if(contextMenu){
                contextMenuPosition = event->pos();
                contextMenu->exec(mapToGlobal(contextMenuPosition));
            }
        }
        else {
            double x1 = this->xAxis->pixelToCoord(xp1);
            double x2 = this->xAxis->pixelToCoord(xp2);
            double y1 = this->yAxis->pixelToCoord(yp1);
            double y2 = this->yAxis->pixelToCoord(yp2);

            this->xAxis->setRange(x1, x2);
            this->yAxis->setRange(y1, y2);
            selectRubberBand->hide();
            this->replot();
        }
        dragAction = DRAG_NONE;
    }
    else if(dragAction == DRAG_CURSOR){
        this->axisRect(0)->setRangeDrag(Qt::Horizontal | Qt::Vertical);
        dragAction = DRAG_NONE;
    }
}

void ExtendedPlot::resetZoom(double x1, double y1, double x2, double y2){
    lazyRangeX[0] = x1;
    lazyRangeX[1] = x2;
    lazyRangeY[0] = y1;
    lazyRangeY[1] = y2;
    this->xAxis->setRange(x1,x2);
    this->yAxis->setRange(y1,y2);
    userZoomed = false;
}

void ExtendedPlot::resetZoom(){
    this->xAxis->setRange(lazyRangeX[0],lazyRangeX[1]);
    this->yAxis->setRange(lazyRangeY[0],lazyRangeY[1]);
    userZoomed = false;
}

ExtendedPlot::~ExtendedPlot(){

}
