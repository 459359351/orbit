// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_QT_TUTORIAL_OVERLAY_
#define ORBIT_QT_TUTORIAL_OVERLAY_

#include <qdialog.h>
#include <qlabel.h>
#include <qpoint.h>

#include "CutoutWidget.h"

namespace Ui {
class TutorialOverlay;
}

class TutorialOverlay : public QDialog {
  Q_OBJECT

 public:
  using InitStep = std::function<void(TutorialOverlay* overlay)>;

  explicit TutorialOverlay(QWidget* parent);
  ~TutorialOverlay() override;

  void ShowStep(int step);
  void NextStep();

  bool eventFilter(QObject* object, QEvent* event) override;

  void SetupStep(int step, QWidget* anchor_widget = nullptr, InitStep callback = nullptr);

 private:
  enum class HintAnchor { kTopLeft, kTopRight, kBottomRight, kBottomLeft };
  struct Hint {
    QWidget* widget = nullptr;
    HintAnchor anchor = HintAnchor::kTopLeft;
    QPoint offset = QPoint(0, 0);
  };

  struct Step {
    InitStep callback = nullptr;
    std::vector<Hint> hints;
    QWidget* anchor_widget = nullptr;
    QWidget* cutout_widget = nullptr;
  };

  std::unique_ptr<Ui::TutorialOverlay> ui_;
  std::vector<std::unique_ptr<QWidget>> border_labels_;
  std::vector<Step> steps_;

  int current_step_ = -1;

  void InitializeStepsFromUi();
  void UpdateEventFilter(QWidget* widget);
  void UpdateGeometry();
  [[nodiscard]] Hint DeriveHintDescription(QRect anchor_rect, QWidget* hint_widget);
  void UpdateHintWidgetPosition(QRect anchor_rect, Hint& hint);

  [[nodiscard]] const QRect AbsoluteGeometry(QWidget* widget);
};

#endif