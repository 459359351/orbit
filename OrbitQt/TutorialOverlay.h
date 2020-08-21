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
  enum class HintAnchor { kTopLeft, kTopRight, kBottomRight, kBottomLeft };

  explicit TutorialOverlay(QWidget* parent);
  ~TutorialOverlay() override;

  void AnchorToWidget(QWidget* widget, HintAnchor anchor_type = HintAnchor::kTopRight,
                      const QPoint& anchor_offset = QPoint(0, 0));

  void ShowStep(int step);
  void NextStep();

  bool eventFilter(QObject* object, QEvent* event) override;

  void AddStep(InitStep step) { steps_.push_back(std::move(step)); }

 private:
  std::unique_ptr<Ui::TutorialOverlay> ui_;
  std::vector<std::unique_ptr<QWidget>> border_labels_;
  std::vector<InitStep> steps_;

  QWidget* anchor_widget_ = nullptr;
  HintAnchor anchor_type_ = HintAnchor::kTopRight;
  QPoint anchor_offset_;

  QLabel* hint_label_ = nullptr;
  CutoutWidget* cutout_ = nullptr;

  int current_step_ = 0;

  void UpdateEventFilter(QWidget* widget);
  void UpdateGeometry();
  [[nodiscard]] QPoint GetLabelBasePosition(QRect widget_rect, QRect label_rect);

  [[nodiscard]] const QRect AbsoluteGeometry(QWidget* widget);
};

#endif