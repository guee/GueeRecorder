#include "FormLayerTools.h"
#include "ui_FormLayerTools.h"
#include "InputSource/ScreenLayer.h"
#include <QStandardItemModel>

FormLayerTools::FormLayerTools(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::FormLayerTools)
{
    ui->setupUi(this);
    QStandardItemModel* model = new QStandardItemModel(this);
    ui->listViewLayers->setModel(model);
    QItemSelectionModel* selModel = new QItemSelectionModel(model);
    ui->listViewLayers->setSelectionModel(selModel);
    connect(selModel, &QItemSelectionModel::currentRowChanged, this, &FormLayerTools::on_listLayersSelect_currentRowChanged);
}

FormLayerTools::~FormLayerTools()
{
    delete ui;
}

void FormLayerTools::enterEvent(QEvent *event)
{
    Q_UNUSED(event)
    setCursor(Qt::ArrowCursor);
    if (m_layer)
    {
        emit selectLayer(m_layer);
    }
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
    setVisible(m_layer);
}

void FormLayerTools::on_pushButtonRemove_clicked()
{
    if (m_layer)
    {
        m_layer->destroy(m_layer);
        m_layer = nullptr;
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

void FormLayerTools::on_pushButtonPrev_clicked()
{
    int32_t i = ui->listViewLayers->currentIndex().row();
    if ( i > 0 )
    {
        QStandardItemModel *model = static_cast<QStandardItemModel*>(ui->listViewLayers->model());
        ui->listViewLayers->setCurrentIndex(model->item(i - 1)->index());
    }
}

void FormLayerTools::on_pushButtonNext_clicked()
{
    int32_t i = ui->listViewLayers->currentIndex().row();
    QStandardItemModel *model = static_cast<QStandardItemModel*>(ui->listViewLayers->model());
    if ( i < model->rowCount() - 1 )
    {
        ui->listViewLayers->setCurrentIndex(model->item(i + 1)->index());
    }
}

void FormLayerTools::on_pushButtonHide_clicked()
{
    //hide();
}

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
        QRectF r = m_layer->rect();
        r.setWidth(arg1 * 1.0 / m_video->width());
        m_layer->setRect(r);
    }
}

void FormLayerTools::on_spinBoxH_valueChanged(int arg1)
{
    if ( m_posChangeByProg ) return;
    if(m_layer)
    {
        QRectF r = m_layer->rect();
        r.setHeight(arg1 * 1.0 / m_video->height());
        m_layer->setRect(r);
    }
}

void FormLayerTools::on_listLayersSelect_currentRowChanged(const QModelIndex &current, const QModelIndex &previous)
{
    Q_UNUSED(previous)
    if (m_posChangeByProg) return;
    if ( current.isValid() )
    {
        m_layer = reinterpret_cast<BaseLayer*>(current.data(Qt::UserRole+1).toULongLong());
        emit selectLayer(m_layer);
        ui->labelLayers->setText(QString("%1/%2").arg(m_layer->layerIndex()).arg(m_video->childLayerCount()));
    }
    else
    {
        m_layer = nullptr;
        emit selectLayer(m_layer);
        ui->labelLayers->setText(QString("-/%2").arg(m_video->childLayerCount()));
    }
    resetButStatus();
    if (m_layer)
    {
        ++m_posChangeByProg;
        ui->spinBoxX->setRange(-m_video->width() * 10, m_video->width());
        ui->spinBoxY->setRange(-m_video->height() * 10, m_video->height());
        ui->spinBoxW->setMaximum(m_video->width() * 10);
        ui->spinBoxH->setMaximum(m_video->height() * 10);

        ui->pushButtonFullScreen->setChecked(m_layer->isFullViewport());
        ui->pushButtonAspratio->setChecked(m_layer->aspectRatioMode() != Qt::IgnoreAspectRatio);
        int32_t i = m_layer->layerIndex();
        ui->pushButtonPrev->setEnabled(i > 0);
        ui->pushButtonNext->setEnabled(i < m_layer->parent()->childLayerCount() - 1);
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
            tit = QString("所有屏幕 (%1 x %2)")
                    .arg(ScreenLayer::screenBound().width())
                    .arg(ScreenLayer::screenBound().height());
            break;
        case ScreenLayer::rectOfScreen:
            ico.load(":/typeIconScreenArea.png");
            tit = QString("屏幕区域 (%1 x %2)")
                    .arg(opt.geometry.width())
                    .arg(opt.geometry.height());
            break;
        case ScreenLayer::specWindow:
            ico.load(":/typeIconWindow.png");
            tit = QString("窗口：%1")
                    .arg(ScreenLayer::windowName(opt.windowId));
            break;
        case ScreenLayer::clientOfWindow:
            ico.load(":/typeIconWindow.png");
            tit = QString("窗口内容：%1")
                    .arg(ScreenLayer::windowName(opt.windowId));
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
    item->setData(QVariant(reinterpret_cast<qlonglong>(layer)));
    model->insertRow(layer->layerIndex(), item);
    if (layer == m_layer )
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
            ui->listViewLayers->setCurrentIndex(model->item(m_layer->layerIndex())->index());
        else
            ui->listViewLayers->setCurrentIndex(QModelIndex());
    }
    setVisible(m_layer);
}

void FormLayerTools::on_layerMoved(BaseLayer *layer)
{
    if (m_layer != layer) return;
    ++m_posChangeByProg;
    ui->spinBoxX->setEnabled(m_layer);
    ui->spinBoxY->setEnabled(m_layer);
    ui->spinBoxW->setEnabled(m_layer);
    ui->spinBoxH->setEnabled(m_layer);
    if(m_layer)
    {
        QRectF rt = m_layer->rect();
        ui->spinBoxX->setValue( qRound(rt.x() * m_video->width()) );
        ui->spinBoxY->setValue( qRound(rt.y() * m_video->height()) );
        ui->spinBoxW->setValue( qRound(rt.width() * m_video->width()) );
        ui->spinBoxH->setValue( qRound(rt.height() * m_video->height()) );
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
            ui->pushButtonPrev->setEnabled(i > 0);
            ui->pushButtonMoveUp->setEnabled(i > 0);
            ui->pushButtonNext->setEnabled(i < m_layer->parent()->childLayerCount() - 1);
            ui->pushButtonMoveDown->setEnabled(i < m_layer->parent()->childLayerCount() - 1);
        }
    }
    else
    {
        ui->pushButtonPrev->setEnabled(false);
        ui->pushButtonMoveUp->setEnabled(false);
        ui->pushButtonNext->setEnabled(false);
        ui->pushButtonMoveDown->setEnabled(false);
    }
}
