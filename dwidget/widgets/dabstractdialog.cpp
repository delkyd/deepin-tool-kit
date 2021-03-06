/**
 * Copyright (C) 2015 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#include <QMouseEvent>
#include <QApplication>
#include <QDesktopWidget>
#include <QPushButton>
#include <QScreen>
#include <QPainter>
#include <QWidget>
#include <QDebug>

#include "dialog_constants.h"
#include "dabstractdialog.h"
#include "private/dabstractdialogprivate_p.h"
#include "dthememanager.h"
#include "dplatformwindowhandle.h"
#include "dapplication.h"

DWIDGET_BEGIN_NAMESPACE

DAbstractDialogPrivate::DAbstractDialogPrivate(DAbstractDialog *qq):
    DObjectPrivate(qq)
{

}

void DAbstractDialogPrivate::init()
{
    D_Q(DAbstractDialog);

    if (qApp->isDXcbPlatform()) {
        handle = new DPlatformWindowHandle(q, q);

        handle->setTranslucentBackground(true);
        handle->setEnableSystemMove(false);
        handle->setEnableSystemResize(false);
    }

    q->setWindowFlags(Qt::FramelessWindowHint  | Qt::WindowCloseButtonHint | Qt::Dialog);
    q->setAttribute(Qt::WA_TranslucentBackground);
    q->resize(DIALOG::DEFAULT_WIDTH, DIALOG::DEFAULT_HEIGHT);
    q->setMaximumWidth(480);
    q->setAttribute(Qt::WA_Resized, false);
    q->setBorderColor(QColor(0, 0, 0));
}

QRect DAbstractDialogPrivate::getParentGeometry() const
{
    D_QC(DAbstractDialog);

    if (q->parentWidget()) {
        return q->parentWidget()->window()->geometry();
    } else {
        QPoint pos = QCursor::pos();

        for (QScreen *screen : qApp->screens()) {
            if (screen->geometry().contains(pos)) {
                return screen->geometry();
            }
        }
    }

    return qApp->primaryScreen()->geometry();
}

DAbstractDialog::DAbstractDialog(QWidget *parent) :
    QDialog(parent),
    DObject(*new DAbstractDialogPrivate(this))
{
    D_THEME_INIT_WIDGET(DAbstractDialog);

    d_func()->init();
}

QColor DAbstractDialog::backgroundColor() const
{
    D_DC(DAbstractDialog);

    return d->backgroundColor;
}

QColor DAbstractDialog::borderColor() const
{
    D_DC(DAbstractDialog);

    return d->borderColor;
}

DAbstractDialog::DisplayPostion DAbstractDialog::displayPostion() const
{
    D_DC(DAbstractDialog);

    return d->displayPostion;
}

void DAbstractDialog::moveToCenter()
{
    D_DC(DAbstractDialog);

    moveToCenterByRect(d->getParentGeometry());
}

void DAbstractDialog::moveToTopRight()
{
    D_DC(DAbstractDialog);

    moveToTopRightByRect(d->getParentGeometry());
}

void DAbstractDialog::moveToTopRightByRect(const QRect &rect)
{
    int x = rect.x() + rect.width() - width();
    move(QPoint(x, 0));
}

void DAbstractDialog::setBackgroundColor(QColor backgroundColor)
{
    D_D(DAbstractDialog);

    d->backgroundColor = backgroundColor;

    update();
}

void DAbstractDialog::setBorderColor(QColor borderColor)
{
    D_D(DAbstractDialog);

    d->borderColor = borderColor;

    if (d->handle) {
        d->handle->setBorderColor(d->borderColor);
    } else {
        update();
    }
}

void DAbstractDialog::setDisplayPostion(DAbstractDialog::DisplayPostion displayPostion)
{
    D_D(DAbstractDialog);

    d->displayPostion = displayPostion;

    switch (displayPostion) {
    case DisplayCenter:
        moveToCenter();
        break;
    case DisplayTopRight:
        moveToTopRight();
        break;
    default:
        break;
    }
}

void DAbstractDialog::moveToCenterByRect(const QRect &rect)
{
    QRect qr = geometry();
    qr.moveCenter(rect.center());
    move(qr.topLeft());
}

void DAbstractDialog::mousePressEvent(QMouseEvent *event)
{
    D_DC(DAbstractDialog);

    if (d->handle) {
        return QDialog::mousePressEvent(event);
    }

    if (event->button() == Qt::LeftButton) {
        D_D(DAbstractDialog);

        d->dragPosition = event->globalPos() - frameGeometry().topLeft();
        d->mousePressed = true;
    }

    QDialog::mousePressEvent(event);
}

void DAbstractDialog::mouseReleaseEvent(QMouseEvent *event)
{
    D_D(DAbstractDialog);

    if (d->handle) {
        return QDialog::mouseReleaseEvent(event);
    }

    d->mousePressed = false;

    QDialog::mouseReleaseEvent(event);
}

void DAbstractDialog::mouseMoveEvent(QMouseEvent *event)
{
    D_D(DAbstractDialog);

    if (d->handle) {
        d->handle->setEnableSystemMove(true);

        return QDialog::mouseMoveEvent(event);
    }

    if (d->mousePressed) {
        move(event->globalPos() - d->dragPosition);
        d->mouseMoved = true;
    }

    QDialog::mouseMoveEvent(event);
}

void DAbstractDialog::paintEvent(QPaintEvent *event)
{
    D_DC(DAbstractDialog);

    QPainter painter(this);

    if (d->handle) {
        painter.fillRect(event->rect(), d->backgroundColor);
    } else {
        painter.setPen(QPen(d->borderColor, DIALOG::BORDER_SHADOW_WIDTH));
        painter.setBrush(d->backgroundColor);
        painter.setRenderHint(QPainter::Antialiasing, true);
        QRectF r(DIALOG::BORDER_SHADOW_WIDTH / 2.0, DIALOG::BORDER_SHADOW_WIDTH / 2.0,
                 width() - DIALOG::BORDER_SHADOW_WIDTH, height() - DIALOG::BORDER_SHADOW_WIDTH);
        painter.drawRoundedRect(r, DIALOG::BORDER_RADIUS, DIALOG::BORDER_RADIUS);
    }

    QDialog::paintEvent(event);
}

void DAbstractDialog::resizeEvent(QResizeEvent *event)
{
    if (event->size().width() >= maximumWidth()) {
        bool resized = testAttribute(Qt::WA_Resized);

        setFixedWidth(maximumWidth());

        if (!resized)
            setAttribute(Qt::WA_Resized, false);
    }
    QDialog::resizeEvent(event);

    D_DC(DAbstractDialog);

    if (!d->mouseMoved) {
        setDisplayPostion(displayPostion());
    }

    emit sizeChanged(event->size());
}

DAbstractDialog::DAbstractDialog(DAbstractDialogPrivate &dd, QWidget *parent):
    QDialog(parent),
    DObject(dd)
{
    dd.init();
}

DWIDGET_END_NAMESPACE
