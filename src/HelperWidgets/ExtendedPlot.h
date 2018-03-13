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
#ifndef _EXTENDED_PLOT_H_
#define _EXTENDED_PLOT_H_

#include "qcustomplot.h"
#include "DataSet.h"

#define GRAPH_COLORS_SIZE   (4)

class ExtendedPlot : public QCustomPlot {
    Q_OBJECT
public:
    static const QColor graph_colors[GRAPH_COLORS_SIZE];
    typedef double (*conv_double_callback_t)(double);

    class Cursor;

    class CursorSpace {
    public:
        QCPItemText* text;
        QCPItemBracket* bracket;
        Cursor* prev;
        Cursor* next;

        CursorSpace(ExtendedPlot* plot,Cursor* left, Cursor* right);
        void remove(ExtendedPlot* plot);
        void setVisible(bool on);
        void update(ExtendedPlot* plot);
        virtual ~CursorSpace();
    };

    class Cursor : public QCPItemLine {
    public:
        static const double big_double;
        enum Type {
            X_AXIS, Y_AXIS
        };

        Type type;
        /* Position in graph coordinates */
        double position;
        QCPItemText* text;
        CursorSpace* spacePrev;
        CursorSpace* spaceNext;

        Cursor(double position, Type type, ExtendedPlot* plot);
        void move(double position,ExtendedPlot* plot);
        virtual void setVisible(bool on);
        void remove(ExtendedPlot* plot);
        virtual ~Cursor();
    };
protected:

    enum DragAction {
        DRAG_NONE,
        DRAG_ZOOM,
        DRAG_CURSOR,
    };

    DragAction dragAction;
    QVector<Cursor*> cursors;
    QVector<CursorSpace*> cursorSpaces;
    Cursor* lastCursorX;
    Cursor* lastCursorY;
    bool cursorsVisible;
    bool userZoomed;
    bool lazyZoomSetting;

    double lazyRangeX[2];
    double lazyRangeY[2];

    QRubberBand *selectRubberBand;
    Cursor* selectedCursorX;
    Cursor* selectedCursorY;
    QPoint selectStartPoint;
    QPoint contextMenuPosition;

    QMenu* contextMenu;
    QAction* resetZoomAction;
    QAction* showPointsAction;
    QAction* storeDataAction;
    QAction* addXCursorAction;
    QAction* addYCursorAction;
    QAction* addXYCursorAction;
    QAction* showHideCursorsAction;
    QAction* removeAllCursorAction;
    double frequencyMultiplier;
protected slots:
    void userZoomChanged(const QCPRange &newRange);

    void selectZoomStart(QMouseEvent *event);
    void selectZoomMove(QMouseEvent *event);
    void selectZoomStop(QMouseEvent *event);
    void addXCursor();
    void addYCursor();
public slots:
    void showPointsCallback(bool show);
    void resetZoomAndReplot();
    void saveData();
    void showHideCursors();
    void removeAllCursors();
    void updateCursorLabels();
public:
    QString xCursorLabel;
    QString yCursorLabel;
    QString xCursorSpaceLabel;
    QString yCursorSpaceLabel;

    void setFrequencyMultiplier(double value);
    void updateCursorLabels(conv_double_callback_t xAxisConvert, conv_double_callback_t yAxisConvert);

    ExtendedPlot(QWidget* parent=0);
    void lazyZoomX(double start, double end);
    void lazyZoomY(double start, double end);
    void resetZoom(double x1, double y1, double x2, double y2);
    void resetZoom();
    void setShowPoints(bool show);
    void setDataSet(DataSet* data);
    QMenu* getContextMenu(){return contextMenu;}
    virtual ~ExtendedPlot();
};

#endif /* _EXTENDED_PLOT_H_ */
