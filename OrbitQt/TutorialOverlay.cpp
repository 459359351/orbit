#include "TutorialOverlay.h"

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
  ui_->errorHint->hide();
  QTabBar* tabBar = ui_->tabWidget->findChild<QTabBar*>();
  tabBar->hide();

  QObject::connect(ui_->btnClose, &QPushButton::clicked, this, &QDialog::close);
  QObject::connect(ui_->btnNext, &QPushButton::clicked, this, &TutorialOverlay::NextStep);
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

void TutorialOverlay::AnchorToWidget(QWidget* widget, HintAnchor anchor_type,
                                     const QPoint& anchor_offset) {
  anchor_offset_ = anchor_offset;
  anchor_type_ = anchor_type;
  UpdateEventFilter(widget);
  UpdateGeometry();
}

void TutorialOverlay::ShowStep(int step) {
  current_step_ = step;

  anchor_widget_ = nullptr;
  anchor_offset_ = QPoint(0, 0);
  hint_label_ = nullptr;

  ui_->errorHint->hide();

  ui_->tabWidget->setCurrentIndex(step);
  hint_label_ = ui_->tabWidget->widget(step)->findChild<QLabel*>();
  cutout_ = ui_->tabWidget->widget(step)->findChild<CutoutWidget*>();
  hint_label_->raise();
  cutout_->raise();

  steps_[current_step_](this);

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
  if (object != anchor_widget_) {
    return false;
  }

  if (event->type() == QEvent::Resize) {
    UpdateGeometry();
  }

  return false;
}

void TutorialOverlay::UpdateEventFilter(QWidget* widget) {
  if (anchor_widget_ != nullptr) {
    anchor_widget_->removeEventFilter(this);
  }

  anchor_widget_ = widget;

  if (anchor_widget_ != nullptr) {
    anchor_widget_->installEventFilter(this);
  }
}

static const auto margin = QPoint(20, 20);

void TutorialOverlay::UpdateGeometry() {
  setGeometry(dynamic_cast<QWidget*>(parent())->rect());

  if (anchor_widget_) {
    auto target_rect = AbsoluteGeometry(anchor_widget_);
    auto outer_rect = target_rect;
    outer_rect.setTopLeft(outer_rect.topLeft() - margin);
    outer_rect.setBottomRight(outer_rect.bottomRight() + margin);

    QRegion maskedRegion(QRegion(QRect(rect())).subtracted(QRegion(target_rect)));
    setMask(maskedRegion);

    cutout_->setGeometry(outer_rect);

    if (hint_label_) {
      const auto label_rect = hint_label_->rect();
      const auto label_pos = GetLabelBasePosition(target_rect, label_rect);
      hint_label_->setGeometry(label_pos.x(), label_pos.y(), label_rect.width(),
                               label_rect.height());
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

QPoint TutorialOverlay::GetLabelBasePosition(QRect widget_rect, QRect label_rect) {
  switch (anchor_type_) {
    case HintAnchor::kTopLeft:
      return widget_rect.topLeft() + QPoint(-margin.x() + anchor_offset_.x(),
                                            -label_rect.height() - margin.y() + anchor_offset_.y());
    case HintAnchor::kTopRight:
      return widget_rect.topRight() +
             QPoint(margin.x() + anchor_offset_.x(),
                    -label_rect.height() - margin.y() + anchor_offset_.y());
    case HintAnchor::kBottomRight:
      return widget_rect.bottomRight() +
             QPoint(margin.x() + anchor_offset_.x(), margin.y() + anchor_offset_.y());
    case HintAnchor::kBottomLeft:
      return widget_rect.bottomLeft() +
             QPoint(-margin.x() + anchor_offset_.x(), margin.y() + anchor_offset_.y());
  }

  UNREACHABLE();
}
