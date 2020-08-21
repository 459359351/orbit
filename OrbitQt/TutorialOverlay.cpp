#include "TutorialOverlay.h"

#include <qframe.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <qregion.h>
#include <qtabbar.h>

#include "OrbitBase/Logging.h"
#include "ui_TutorialOverlay.h"

TutorialOverlay::TutorialOverlay(QWidget* parent)
    : QDialog(parent), ui_(std::make_unique<Ui::TutorialOverlay>()) {
  ui_->setupUi(this);

  setGeometry(QRect(0, 0, parent->rect().width(), parent->rect().height()));
  setWindowFlags(Qt::FramelessWindowHint | Qt::WindowSystemMenuHint);

  for (int i = 0; i < 4; ++i) {
    auto label = std::make_unique<QLabel>(this);
    label->setStyleSheet("background-color: rgba(0, 0, 0, 150);");
    border_labels_.push_back(std::move(label));
  }

  ui_->tabWidget->raise();
  ui_->bottomBar->raise();
  ui_->bottomBar->hide();
  ui_->errorHint->hide();
  QTabBar* tabBar = ui_->tabWidget->findChild<QTabBar*>();
  tabBar->hide();

  InitializeStepsFromUi();

  QObject::connect(ui_->btnClose, &QPushButton::clicked, this, &QDialog::close);
  QObject::connect(ui_->btnNext, &QPushButton::clicked, this, &TutorialOverlay::NextStep);
}

void TutorialOverlay::InitializeStepsFromUi() {
  for (int tab_i = 0; tab_i < ui_->tabWidget->count(); ++tab_i) {
    QWidget* tab = ui_->tabWidget->widget(tab_i);

    Step step;
    step.cutout_widget = tab->findChild<QLabel*>();
    if (step.cutout_widget) {
      QRect cutout_rect = step.cutout_widget->geometry();
      for (auto frame : tab->findChildren<QFrame*>()) {
        if (frame != step.cutout_widget && frame->parent() == tab) {
          frame->setStyleSheet("QFrame { border: 0; }");
          step.hints.push_back(DeriveHintDescription(cutout_rect, frame));
        }
      }
    }

    steps_.push_back(std::move(step));
  }
}

const QRect TutorialOverlay::AbsoluteGeometry(QWidget* widget) {
  if (widget->parent() == nullptr || !widget->parent()->isWidgetType() ||
      widget->parent() == parent()) {
    return widget->geometry();
  }

  QWidget* parent_widget = dynamic_cast<QWidget*>(widget->parent());

  QRect result = widget->geometry();
  QRect parent_geo = AbsoluteGeometry(parent_widget);

  auto width = result.width();
  auto height = result.height();

  result.setX(result.x() + parent_geo.x());
  result.setY(result.y() + parent_geo.y());

  result.setWidth(width);
  result.setHeight(height);

  return result;
}

TutorialOverlay::~TutorialOverlay() {}

void TutorialOverlay::ShowStep(int step) {
  if (current_step_ >= 0) {
    if (steps_[current_step_].anchor_widget != nullptr) {
      steps_[current_step_].anchor_widget->removeEventFilter(this);
    }
  }

  current_step_ = step;
  ui_->errorHint->hide();

  ui_->tabWidget->setCurrentIndex(step);

  if (steps_[current_step_].callback) {
    steps_[current_step_].callback(this);
  }

  UpdateEventFilter(steps_[current_step_].anchor_widget);
  showMaximized();
  UpdateGeometry();
}

void TutorialOverlay::NextStep() {
  if (current_step_ < steps_.size() - 1) {
    ShowStep(current_step_ + 1);
  } else {
    close();
  }
}

bool TutorialOverlay::eventFilter(QObject* object, QEvent* event) {
  if (object != steps_[current_step_].anchor_widget) {
    return false;
  }

  if (event->type() == QEvent::Resize) {
    UpdateGeometry();
  }

  return false;
}

void TutorialOverlay::SetupStep(int step, QWidget* anchor_widget, InitStep callback) {
  CHECK(step >= 0);
  CHECK(step < steps_.size());

  steps_[step].anchor_widget = anchor_widget;
  steps_[step].callback = callback;
}

void TutorialOverlay::UpdateEventFilter(QWidget* widget) {
  if (widget == nullptr) {
    return;
  }

  widget->installEventFilter(this);
}

static const auto margin = QPoint(20, 20);

void TutorialOverlay::UpdateGeometry() {
  setGeometry(dynamic_cast<QWidget*>(parent())->rect());
  Step& step = steps_[current_step_];

  if (step.anchor_widget != nullptr && step.cutout_widget != nullptr) {
    auto target_rect = AbsoluteGeometry(step.anchor_widget);
    auto outer_rect = target_rect;
    outer_rect.setTopLeft(outer_rect.topLeft() - margin);
    outer_rect.setBottomRight(outer_rect.bottomRight() + margin);

    QRegion maskedRegion(QRegion(QRect(rect())).subtracted(QRegion(target_rect)));
    setMask(maskedRegion);

    step.cutout_widget->setGeometry(outer_rect);

    for (auto& hint : step.hints) {
      UpdateHintWidgetPosition(outer_rect, hint);
    }

    border_labels_[0]->setGeometry(0, 0, rect().width(), target_rect.top());
    border_labels_[1]->setGeometry(0, target_rect.bottom(), rect().width(),
                                   rect().height() - target_rect.bottom());
    border_labels_[2]->setGeometry(0, target_rect.top(), target_rect.left(), target_rect.height());
    border_labels_[3]->setGeometry(target_rect.right(), target_rect.top(),
                                   rect().width() - target_rect.right(), target_rect.height());
  } else {
    border_labels_[0]->setGeometry(rect());
    border_labels_[1]->setGeometry(0, 0, 0, 0);
    border_labels_[2]->setGeometry(0, 0, 0, 0);
    border_labels_[3]->setGeometry(0, 0, 0, 0);
  }
}

TutorialOverlay::Hint TutorialOverlay::DeriveHintDescription(QRect anchor_rect,
                                                             QWidget* hint_widget) {
  QRect hint_rect = hint_widget->geometry();

  // The anchor position is determined by the quadrant of anchor_rect in which the top
  // left corner of hint_rect is placed initially.
  Hint result;
  result.widget = hint_widget;

  if (hint_rect.x() < anchor_rect.x() + anchor_rect.width() / 2) {
    if (hint_rect.y() < anchor_rect.y() + anchor_rect.height() / 2) {
      result.anchor = HintAnchor::kTopLeft;
    } else {
      result.anchor = HintAnchor::kBottomLeft;
    }
  } else {
    if (hint_rect.y() < anchor_rect.y() + anchor_rect.height() / 2) {
      result.anchor = HintAnchor::kTopRight;
    } else {
      result.anchor = HintAnchor::kBottomRight;
    }
  }

  switch (result.anchor) {
    // Anchored top left: Top right label corner is reference
    case HintAnchor::kTopLeft:
      result.offset = hint_rect.topRight() - anchor_rect.topLeft();
      break;
    // Anchored top right: Top left label corner is reference
    case HintAnchor::kTopRight:
      result.offset = hint_rect.topLeft() - anchor_rect.topRight();
      break;
    // Anchored bottom right: Top left label corner is reference
    case HintAnchor::kBottomRight:
      result.offset = hint_rect.topLeft() - anchor_rect.bottomRight();
      break;
    // Anchored bottom left: Top right label corner is reference
    case HintAnchor::kBottomLeft:
      result.offset = hint_rect.topRight() - anchor_rect.bottomLeft();
      break;
  }

  return result;
}

void TutorialOverlay::UpdateHintWidgetPosition(QRect anchor_rect, Hint& hint) {
  const QPoint size(hint.widget->rect().width(), hint.widget->rect().height());
  QPoint tl, br, tr, bl;

  // See comments in DeriveHintDescription() on how offsets and anchors
  // are calculated
  switch (hint.anchor) {
    case HintAnchor::kTopLeft:
      tr = anchor_rect.topLeft() + hint.offset;
      tl = tr - QPoint(0, size.y());
      break;
    case HintAnchor::kTopRight:
      tl = anchor_rect.topRight() + hint.offset;
      break;
    case HintAnchor::kBottomRight:
      tl = anchor_rect.bottomRight() + hint.offset;
      break;
    case HintAnchor::kBottomLeft:
      tr = anchor_rect.bottomLeft() + hint.offset;
      tl = tr - QPoint(0, size.y());
      break;
  }

  br = tl + size;
  hint.widget->setGeometry(QRect(tl, br));
}
