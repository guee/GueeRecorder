#include "FormLayerTools.h"
#include "ui_FormLayerTools.h"
#include "InputSource/ScreenLayer.h"
#include <QStandardItemModel>
#include "DialogSetting.h"

FormLayerTools::FormLayerTools(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::FormLayerTools)
{
    ui->setupUi(this);
    ui->listViewLayers->setAlternatingRowColors(true);
    QStandardItemModel* model = new QStandardItemModel(this);
    ui->listViewLayers->setModel(model);
    QItemSelectionModel* selModel = new QItemSelectionModel(model);
    ui->listViewLayers->setSelectionModel(selModel);
    connect(selModel, &QItemSelectionModel::currentRowChanged, this, &FormLayerTools::on_listLayersSelect_currentRowChanged);
    setCursor(Qt::ArrowCursor);
    startTimer(200);
}

FormLayerTools::~FormLayerTools()
{
    delete ui;
}

void FormLayerTools::setStyleIsLeft(bool isLeft)
{
    if (isLeft)
    {
        ui->widgetBox->setStyleSheet("QWidget#widgetBox{background-color: qlineargradient(spread:pad, x1:0, y1:0, x2:1, y2:0, stop:0 rgba(0, 0, 0, 255), stop:1 rgba(79, 79, 79, 196));}");
    }
    else
    {
        ui->widgetBox->setStyleSheet("QWidget#widgetBox{background-color: qlineargradient(spread:pad, x1:1, y1:0, x2:0, y2:0, stop:0 rgba(0, 0, 0, 255), stop:1 rgba(79, 79, 79, 196));}");
    }
}

void FormLayerTools::refreshLayers(VideoSynthesizer *videoObj)
{
    m_video = videoObj;
    int32_t count = m_video->childLayerCount();
    QStandardItemModel *model = static_cast<QStandardItemModel*>(ui->listViewLayers->model());

    model->clear();
    if (m_video == nullptr) return;
    for ( int32_t i = count - 1; i >= 0; --i)
    {
        BaseLayer* layer = m_video->childLayer(i);
        on_layerAdded(layer);
    }
}

bool FormLayerTools::windowIsPeg()
{
    return ui->pushButtonDing->isChecked();
}

bool FormLayerTools::event(QEvent *event)
{
    if (event->type() == QEvent::Enter)
    {
        if (ui->pushButtonDing->isChecked())
        {
            qDebug() << "QEvent::Enter:" << m_layer;
            emit selectLayer(m_layer);
        }
    }
    if (event->type() == QEvent::ToolTip
            || event->type() == QEvent::MouseMove
            || event->type() == QEvent::MouseButtonPress
            || event->type() == QEvent::MouseButtonRelease
            || event->type() == QEvent::Enter
            || event->type() == QEvent::Leave)
    {
        event->accept();
        return true;
    }
    return QWidget::event(event);
}

void FormLayerTools::setFixedWindow(bool fixed)
{
    ui->pushButtonDing->setChecked(fixed);
}

void FormLayerTools::timerEvent(QTimerEvent *event)
{
    if (m_layer)
    {
        bool isVisabled = (m_layer->hasImage() && m_layer->isVisabled());
        if (m_curLayerIsVisabled != isVisabled)
        {
            m_curLayerIsVisabled = isVisabled;
            ui->scrollAreaOption->setEnabled(m_curLayerIsVisabled);
            emit selectLayer(m_layer);
        }
    }
}

void FormLayerTools::on_pushButtonRemove_clicked()
{
    if (m_layer)
    {
        BaseLayer* delLay = m_layer;
        m_layer = nullptr;
        delLay->destroy(delLay);
    }
}

void FormLayerTools::on_pushButtonFullScreen_clicked(bool checked)
{
    if(m_layer)
    {
        m_layer->fullViewport(checked);
    }
}

void FormLayerTools::on_pushButtonAspratio_clicked(bool checked)
{
    if(m_layer)
    {
        m_layer->setAspectRatioMode( checked ? Qt::KeepAspectRatio : Qt::IgnoreAspectRatio);
    }
}

void FormLayerTools::on_pushButtonMoveUp_clicked()
{
    if ( m_layer )
    {
        int32_t i = ui->listViewLayers->currentIndex().row();
        if ( i > 0 )
        {
            m_layer->setParent(m_layer->parent(), i - 1);
            QStandardItemModel *model = static_cast<QStandardItemModel*>(ui->listViewLayers->model());
            QStandardItem* item = model->takeItem(i);
            model->removeRow(i);
            model->insertRow(i - 1, item);
            ui->listViewLayers->setCurrentIndex(item->index());
        }
    }
}

void FormLayerTools::on_pushButtonMoveDown_clicked()
{
    if ( m_layer )
    {
        int32_t i = ui->listViewLayers->currentIndex().row();
        QStandardItemModel *model = static_cast<QStandardItemModel*>(ui->listViewLayers->model());
        if ( i < model->rowCount() - 1 )
        {
            m_layer->setParent(m_layer->parent(), i + 1);
            QStandardItem* item = model->takeItem(i);
            model->removeRow(i);
            model->insertRow(i + 1, item);
            ui->listViewLayers->setCurrentIndex(item->index());

        }
    }
}

//void FormLayerTools::on_pushButtonPrev_clicked()
//{
//    int32_t i = ui->listViewLayers->currentIndex().row();
//    if ( i > 0 )
//    {
//        QStandardItemModel *model = static_cast<QStandardItemModel*>(ui->listViewLayers->model());
//        ui->listViewLayers->setCurrentIndex(model->item(i - 1)->index());
//    }
//}

//void FormLayerTools::on_pushButtonNext_clicked()
//{
//    int32_t i = ui->listViewLayers->currentIndex().row();
//    QStandardItemModel *model = static_cast<QStandardItemModel*>(ui->listViewLayers->model());
//    if ( i < model->rowCount() - 1 )
//    {
//        ui->listViewLayers->setCurrentIndex(model->item(i + 1)->index());
//    }
//}


void FormLayerTools::on_spinBoxX_valueChanged(int arg1)
{
    if ( m_posChangeByProg ) return;
    if(m_layer)
    {
        QRectF r = m_layer->rect();
        r.moveLeft(arg1 * 1.0 / m_video->width());
        m_layer->setRect(r);
    }
}

void FormLayerTools::on_spinBoxY_valueChanged(int arg1)
{
    if ( m_posChangeByProg ) return;
    if(m_layer)
    {
        QRectF r = m_layer->rect();
        r.moveTop(arg1 * 1.0 / m_video->height());
        m_layer->setRect(r);
    }
}

void FormLayerTools::on_spinBoxW_valueChanged(int arg1)
{
    if ( m_posChangeByProg ) return;
    if(m_layer)
    {
        m_layer->setWidth(arg1 * 1.0 / m_video->width());
    }
}

void FormLayerTools::on_spinBoxH_valueChanged(int arg1)
{
    if ( m_posChangeByProg ) return;
    if(m_layer)
    {
        m_layer->setHeight(arg1 * 1.0 / m_video->height());
    }
}

void FormLayerTools::on_listLayersSelect_currentRowChanged(const QModelIndex &current, const QModelIndex &previous)
{
    Q_UNUSED(previous)
    if (m_posChangeByProg) return;
    if ( current.isValid() )
    {
        m_layer = reinterpret_cast<BaseLayer*>(current.data(Qt::UserRole+1).toULongLong());
        m_curLayerIsVisabled = m_layer->hasImage() && m_layer->isVisabled();
        qDebug() << "on_listLayersSelect_currentRowChanged(isValid):" << m_layer;
        emit selectLayer(m_layer);
        ui->labelLayers->setText(QString("%1/%2").arg(m_layer->layerIndex() + 1).arg(m_video->childLayerCount()));
        ui->scrollAreaOption->setEnabled(m_curLayerIsVisabled);
        ui->widgetLayerTools->setEnabled(true);
    }
    else
    {
        m_layer = nullptr;
        qDebug() << "on_listLayersSelect_currentRowChanged(no valid):" << m_layer;
        emit selectLayer(m_layer);
        ui->labelLayers->setText(QString("-/%2").arg(m_video->childLayerCount()));
        ui->scrollAreaOption->setEnabled(false);
        ui->widgetLayerTools->setEnabled(false);

    }
    resetButStatus();
    if (m_layer)
    {
        ++m_posChangeByProg;
        ui->spinBoxX->setRange(-m_video->width() * 10, m_video->width() * 10);
        ui->spinBoxY->setRange(-m_video->height() * 10, m_video->height() * 10);
        ui->spinBoxW->setMaximum(65535);
        ui->spinBoxH->setMaximum(65535);

        ui->pushButtonFullScreen->setChecked(m_layer->isFullViewport());
        ui->pushButtonAspratio->setChecked(m_layer->aspectRatioMode() != Qt::IgnoreAspectRatio);

        ui->horizontalSliderHue->setValue(m_layer->imgHue() * float(ui->horizontalSliderHue->maximum()) * 2.0f);
        ui->checkBoxTinting->setChecked(m_layer->imgHueDye());
        ui->horizontalSliderSaturability->setValue(m_layer->imgSaturation() * float(ui->horizontalSliderSaturability->maximum()));
        ui->horizontalSliderLuminance->setValue(m_layer->imgBright() * float(ui->horizontalSliderLuminance->maximum()));
        ui->horizontalSliderContrast->setValue(m_layer->imgContrast() * float(ui->horizontalSliderContrast->maximum()));
        ui->horizontalSliderTransparence->setValue(m_layer->imgTransparence() * float(ui->horizontalSliderTransparence->maximum()));


        int32_t i = m_layer->layerIndex();

        ui->pushButtonMoveUp->setEnabled(i > 0);
        ui->pushButtonMoveDown->setEnabled(i < m_layer->parent()->childLayerCount() - 1);
        --m_posChangeByProg;
    }
    on_layerMoved(m_layer);
}

void FormLayerTools::on_layerAdded(BaseLayer *layer)
{
    QStandardItemModel *model = static_cast<QStandardItemModel*>(ui->listViewLayers->model());
    QPixmap ico;
    QString tit;
    if (layer->layerType() == "screen")
    {
        ScreenLayer::Option opt = dynamic_cast<ScreenLayer*>(layer)->shotOption();
        switch (opt.mode)
        {
        case ScreenLayer::unspecified:
            break;
        case ScreenLayer::specScreen:
            ico.load(":/typeIconScreen.png");
            tit = QString("屏幕 %1 (%2 x %3)").arg(opt.screenIndex + 1)
                    .arg(ScreenLayer::screenRect(opt.screenIndex).width())
                    .arg(ScreenLayer::screenRect(opt.screenIndex).height());
            break;
        case ScreenLayer::fullScreen:
            ico.load(":/typeIconAllScreen.png");
            tit = QString("全部 (%1 x %2)")
                    .arg(ScreenLayer::screenBound().width())
                    .arg(ScreenLayer::screenBound().height());
            break;
        case ScreenLayer::rectOfScreen:
            ico.load(":/typeIconScreenArea.png");
            tit = QString("区域 (%1 x %2)")
                    .arg(opt.geometry.width())
                    .arg(opt.geometry.height());
            break;
        case ScreenLayer::specWindow:
            ico.load(":/typeIconWindow.png");
            tit = QString("%1")
                    .arg(ScreenLayer::windowName(opt.widReal));
            break;
        case ScreenLayer::clientOfWindow:
            ico.load(":/typeIconWindow.png");
            tit = QString("%1")
                    .arg(ScreenLayer::windowName(opt.widReal));
            break;
        }
    }
    else if (layer->layerType() == "picture")
    {
        ico.load(":/typeIconPicture.png");
        tit = layer->sourceName();
    }
    else if (layer->layerType() == "camera")
    {
        ico.load(":/typeIconCamera.png");
        tit = layer->sourceName();
    }
    QStandardItem* item = new QStandardItem(QIcon(ico), tit);
    //QStandardItem* item = new QStandardItem(tit);
    //item->setCheckable(true);
    //item->setCheckState(Qt::Checked);
    item->setData(QVariant(reinterpret_cast<qlonglong>(layer)));
    //item->setData(Qt::Checked, Qt::CheckStateRole);
    model->insertRow(layer->layerIndex(), item);
   // ui->listViewLayers->setIndexWidget()
    //model->setI
    if (nullptr == m_layer )
    {
        ui->listViewLayers->setCurrentIndex(item->index());
    }
    if (m_layer)
    {
        ui->labelLayers->setText(QString("%1/%2").arg(m_layer->layerIndex()).arg(m_video->childLayerCount()));
    }
    else
    {
        ui->labelLayers->setText(QString("-/%2").arg(m_video->childLayerCount()));
    }
}

void FormLayerTools::on_layerRemoved(BaseLayer *layer)
{
    QStandardItemModel *model = static_cast<QStandardItemModel*>(ui->listViewLayers->model());
    if (m_layer == layer)
    {
        m_layer = nullptr;
    }
    for (int i = 0; i < model->rowCount(); ++i)
    {
        if (model->item(i)->data().toLongLong() == reinterpret_cast<qlonglong>(layer))
        {
            model->removeRow(i);
            break;
        }
    }
}

void FormLayerTools::on_selectLayer(BaseLayer *layer)
{
    if (m_layer != layer)
    {
        m_layer = layer;

        QStandardItemModel *model = static_cast<QStandardItemModel*>(ui->listViewLayers->model());
        if (m_layer)
        {
            ui->listViewLayers->setCurrentIndex(model->item(m_layer->layerIndex())->index());
            m_curLayerIsVisabled = m_layer->hasImage() && m_layer->isVisabled();
        }
        else
            ui->listViewLayers->setCurrentIndex(QModelIndex());
    }
}

void FormLayerTools::on_layerMoved(BaseLayer *layer)
{
    if (m_layer != layer) return;
    ++m_posChangeByProg;

    //ui->pushButtonFullScreen->setEnabled(m_layer);
    if(m_layer)
    {
        QRectF rt = m_layer->rect();
        ui->spinBoxX->setValue( qRound(rt.x() * m_video->width()) );
        ui->spinBoxY->setValue( qRound(rt.y() * m_video->height()) );
        ui->spinBoxW->setValue( qRound(rt.width() * m_video->width()) );
        ui->spinBoxH->setValue( qRound(rt.height() * m_video->height()) );
        ui->pushButtonFullScreen->setChecked(m_layer->isFullViewport());
    }

    --m_posChangeByProg;
}

int32_t FormLayerTools::findLayerItem(BaseLayer *layer)
{
    QStandardItemModel *model = static_cast<QStandardItemModel*>(ui->listViewLayers->model());
    int32_t index = -1;
    if ( model )
    {
        for ( int32_t i = 0; i < model->rowCount(); ++i )
        {
            if ( layer == static_cast<ScreenLayer*>(model->item(i)->data().data()) )
            {
                index = i;
                break;
            }
        }
    }
    return index;

}

void FormLayerTools::resetButStatus()
{
    if(m_layer)
    {
        int32_t i = m_layer->layerIndex();
        if ( i >= 0 )
        {

            ui->pushButtonMoveUp->setEnabled(i > 0);

            ui->pushButtonMoveDown->setEnabled(i < m_layer->parent()->childLayerCount() - 1);
        }
    }
    else
    {

        ui->pushButtonMoveUp->setEnabled(false);

        ui->pushButtonMoveDown->setEnabled(false);
    }
}

void FormLayerTools::on_pushButtonDing_clicked(bool checked)
{
    DialogSetting::userSetting().fixedLayWnd = checked;
    DialogSetting::saveProfile();
    if (checked == false && m_layer == nullptr)
    {
        qDebug() << "on_pushButtonDing_clicked:" << m_layer;
        emit selectLayer(m_layer);
    }
}

void FormLayerTools::on_horizontalSliderHue_valueChanged(int value)
{
    if(m_layer)
    {
        m_layer->setImgHue(float(value) * 0.5f / float(ui->horizontalSliderHue->maximum()));
    }
}

void FormLayerTools::on_checkBoxTinting_clicked(bool checked)
{
    if(m_layer)
    {
        m_layer->setImgHueDye(checked);
    }
}

void FormLayerTools::on_horizontalSliderSaturability_valueChanged(int value)
{
    if(m_layer)
    {
        m_layer->setImgSaturation(float(value) / float(ui->horizontalSliderSaturability->maximum()));
    }
}

void FormLayerTools::on_horizontalSliderLuminance_valueChanged(int value)
{
    if(m_layer)
    {
        m_layer->setImgBright(float(value) / float(ui->horizontalSliderLuminance->maximum()));
    }
}

void FormLayerTools::on_horizontalSliderContrast_valueChanged(int value)
{
    if(m_layer)
    {
        m_layer->setImgContrast(float(value) / float(ui->horizontalSliderContrast->maximum()));
    }
}

void FormLayerTools::on_horizontalSliderTransparence_valueChanged(int value)
{
    if(m_layer)
    {
        m_layer->setImgTransparence(float(value) / float(ui->horizontalSliderTransparence->maximum()));
    }
}

void FormLayerTools::on_groupBox_ImageColor_clicked(bool checked)
{
    ui->horizontalSliderHue->setValue(0);
    ui->checkBoxTinting->setChecked(false);
    ui->horizontalSliderSaturability->setValue(0);
    ui->horizontalSliderLuminance->setValue(0);
    ui->horizontalSliderContrast->setValue(0);
    ui->horizontalSliderTransparence->setValue(0);

    if(m_layer)
    {
        m_layer->setImgHue(0.0f);
        m_layer->setImgHueDye(false);
        m_layer->setImgSaturation(0.0f);
        m_layer->setImgBright(0.0f);
        m_layer->setImgContrast(0.0f);
        m_layer->setImgTransparence(0.0f);
    }
    ui->groupBox_ImageColor->setChecked(true);
}
