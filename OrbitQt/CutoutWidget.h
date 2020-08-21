// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_QT_CUTOUT_WIDGET_
#define ORBIT_QT_CUTOUT_WIDGET_

#include <QLabel>

class CutoutWidget : public QLabel {
  Q_OBJECT

 public:
  explicit CutoutWidget(QWidget* parent = nullptr) : QLabel{parent} {}
};

#endif
