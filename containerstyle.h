/*
 * Copyright (C) 2017 Zvaigznu Planetarijs
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see http://www.gnu.org/licenses/.
 *
 */

#pragma once

//
// includes
//
#include <QProxyStyle>

/**
 * @brief The ContainerStyle class - hides ugly drop indicator
 */
class ContainerStyle : public QProxyStyle {
public:
    ContainerStyle( QStyle *style = nullptr ) : QProxyStyle( style ) {}
    void drawPrimitive( PrimitiveElement element, const QStyleOption *option, QPainter *painter, const QWidget *widget ) const {
        if ( element != QStyle::PE_IndicatorItemViewItemDrop )
            QProxyStyle::drawPrimitive( element, option, painter, widget );
    }
};
