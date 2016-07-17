#include "musicsongstoolboxwidget.h"
#include "musicclickedlabel.h"
#include "musicuiobject.h"
#include "musicobject.h"
#include "musicsongstoolitemrenamedwidget.h"

#include <QMenu>
#include <QPainter>
#include <QScrollArea>
#include <QMouseEvent>

MusicSongsToolBoxTopWidget::MusicSongsToolBoxTopWidget(int index, const QString &text, QWidget *parent)
    : QWidget(parent)
{
    m_renameLine = nullptr;
    m_index = index;
    setFixedHeight(35);

    QHBoxLayout *topLayout = new QHBoxLayout(this);
    topLayout->setContentsMargins(3, 0, 0, 0);
    topLayout->setSpacing(0);
    m_labelIcon = new QLabel(this);
    m_labelIcon->setPixmap(QPixmap(":/image/arrowup"));
    m_labelText = new QLabel(this);
    m_labelText->setText(text);
    MusicClickedLabel *menuLabel = new MusicClickedLabel(this);
    connect(menuLabel, SIGNAL(clicked()), SLOT(showMenu()));
    menuLabel->setPixmap(QPixmap(":/toolSets/listmenu"));
    topLayout->addWidget(m_labelIcon);
    topLayout->addWidget(m_labelText, 20);
    topLayout->addWidget(menuLabel);
    topLayout->addStretch(1);
    setLayout(topLayout);
}

MusicSongsToolBoxTopWidget::~MusicSongsToolBoxTopWidget()
{
    delete m_renameLine;
    delete m_labelIcon;
    delete m_labelText;
}

QString MusicSongsToolBoxTopWidget::getClassName()
{
    return staticMetaObject.className();
}

void MusicSongsToolBoxTopWidget::setItemExpand(bool expand)
{
    m_labelIcon->setPixmap(QPixmap(expand ? ":/image/arrowdown" : ":/image/arrowup"));
}

void MusicSongsToolBoxTopWidget::setTitle(const QString &text)
{
    m_labelText->setText(text);
}

QString MusicSongsToolBoxTopWidget::getTitle() const
{
    return m_labelText->text().trimmed();
}

void MusicSongsToolBoxTopWidget::deleteRowItem()
{
    emit deleteRowItem(m_index);
}

void MusicSongsToolBoxTopWidget::changRowItemName()
{
    if(!m_renameLine)
    {
        m_renameLine = new MusicSongsToolItemRenamedWidget(getTitle(), this);
        connect(m_renameLine, SIGNAL(renameFinished(QString)), SLOT(setChangItemName(QString)));
        m_renameLine->setGeometry(m_labelIcon->width(), 3, 250, height() - 6);
    }
    m_renameLine->show();
}

void MusicSongsToolBoxTopWidget::setChangItemName(const QString &name)
{
    m_labelText->setText(name);
    m_labelText->setToolTip(name);
    emit renameFinished(m_index, name);

    m_renameLine->deleteLater();
    m_renameLine = nullptr;
}

void MusicSongsToolBoxTopWidget::showMenu()
{
    QMenu menu(this);
    menu.setStyleSheet(MusicUIObject::MMenuStyle02);
    menu.addAction(tr("addNewItem"), parent(), SIGNAL(addNewRowItem()));
    menu.addSeparator();

    bool disable = !(m_index == 0 || m_index == 1 || m_index == 2);
    menu.addAction(tr("deleteItem"), this, SLOT(deleteRowItem()))->setEnabled(disable);
    menu.addAction(tr("changItemName"), this, SLOT(changRowItemName()))->setEnabled(disable);
    menu.exec(QCursor::pos());
}

void MusicSongsToolBoxTopWidget::mousePressEvent(QMouseEvent *event)
{
    QWidget::mousePressEvent(event);
    if(event->button() == Qt::LeftButton)
    {
        emit mousePressAt(m_index);
    }
}

void MusicSongsToolBoxTopWidget::paintEvent(QPaintEvent *event)
{
    QWidget::paintEvent(event);
    QPainter painter(this);
    painter.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
    painter.setPen(QPen(QBrush(QColor(0, 0, 0)), 0.1, Qt::SolidLine));
    painter.drawLine(0, height(), width(), height());
}

void MusicSongsToolBoxTopWidget::contextMenuEvent(QContextMenuEvent *event)
{
    QWidget::contextMenuEvent(event);
    showMenu();
}


MusicSongsToolBoxWidgetItem::MusicSongsToolBoxWidgetItem(int index, const QString &text, QWidget *parent)
    : QWidget(parent)
{
    m_topWidget = new MusicSongsToolBoxTopWidget(index, text, this);
    connect(m_topWidget, SIGNAL(mousePressAt(int)), parent, SLOT(mousePressAt(int)));
    connect(m_topWidget, SIGNAL(deleteRowItem(int)), SIGNAL(deleteRowItem(int)));
    connect(m_topWidget, SIGNAL(renameFinished(int,QString)), SIGNAL(changRowItemName(int,QString)));

    m_layout = new QVBoxLayout(this);
    m_layout->setContentsMargins(0, 0, 0, 0);
    m_layout->setSpacing(0);
    m_layout->addWidget(m_topWidget);
    setLayout(m_layout);
}

MusicSongsToolBoxWidgetItem::~MusicSongsToolBoxWidgetItem()
{
    delete m_topWidget;
    delete m_layout;
}

QString MusicSongsToolBoxWidgetItem::getClassName()
{
    return staticMetaObject.className();
}

QWidget *MusicSongsToolBoxWidgetItem::item(int index)
{
    if(index < 0 || index >= m_itemList.count())
    {
        return nullptr;
    }
    return m_itemList[index];
}

void MusicSongsToolBoxWidgetItem::addItem(QWidget *item)
{
    m_itemList.append(item);
    m_layout->addWidget(item);
}

void MusicSongsToolBoxWidgetItem::removeItem(QWidget *item)
{
    m_itemList.removeAll(item);
    m_layout->removeWidget(item);
}

void MusicSongsToolBoxWidgetItem::setItemHide(bool hide)
{
    m_topWidget->setItemExpand(hide);
    foreach(QWidget *w, m_itemList)
    {
        w->setVisible(hide);
    }
}

bool MusicSongsToolBoxWidgetItem::itemHide() const
{
    if(!m_itemList.isEmpty())
    {
        return m_itemList.first()->isVisible();
    }
    return false;
}

void MusicSongsToolBoxWidgetItem::mousePressEvent(QMouseEvent *event)
{
    Q_UNUSED(event);
}

void MusicSongsToolBoxWidgetItem::contextMenuEvent(QContextMenuEvent *event)
{
    Q_UNUSED(event);
}


MusicSongsToolBoxWidget::MusicSongsToolBoxWidget(QWidget *parent)
    : QWidget(parent)
{
    setAttribute(Qt::WA_TranslucentBackground);

    m_currentIndex = -1;
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    QWidget *contentsWidget = new QWidget(this);
    m_layout = new QVBoxLayout(contentsWidget);
    m_layout->setContentsMargins(0, 0, 0 ,0);
    m_layout->setSpacing(0);
    contentsWidget->setLayout(m_layout);

    QScrollArea *scrollArea = new QScrollArea(this);
    scrollArea->setStyleSheet(MusicUIObject::MScrollBarStyle01);
    scrollArea->setWidgetResizable(true);
    scrollArea->setFrameShape(QFrame::NoFrame);
    scrollArea->setAlignment(Qt::AlignLeft);
    scrollArea->setWidget(contentsWidget);

    QString style = "background:rgba(255,255,255,25)";
    contentsWidget->setObjectName("contentsWidget");
    contentsWidget->setStyleSheet(QString("#contentsWidget{%1}").arg(style));
    QWidget *view = scrollArea->viewport();
    view->setObjectName("viewport");
    view->setStyleSheet(QString("#viewport{%1}").arg(style));

    mainLayout->addWidget(scrollArea);
    setLayout(mainLayout);
}

MusicSongsToolBoxWidget::~MusicSongsToolBoxWidget()
{
    while(!m_itemList.isEmpty())
    {
        delete m_itemList.takeLast();
    }
    delete m_layout;
}

QString MusicSongsToolBoxWidget::getClassName()
{
    return staticMetaObject.className();
}

void MusicSongsToolBoxWidget::setCurrentIndex(int index)
{
    m_currentIndex = index;
    for(int i=0; i<m_itemList.count(); ++i)
    {
        m_itemList[i]->setItemHide( i== index );
    }
}

void MusicSongsToolBoxWidget::mousePressAt(int index)
{
    m_currentIndex = index;
    for(int i=0; i<m_itemList.count(); ++i)
    {
        bool hide = (i== index) ? !m_itemList[i]->itemHide() : false;
        m_itemList[i]->setItemHide(hide);
    }
}

int MusicSongsToolBoxWidget::currentIndex() const
{
    return m_currentIndex;
}

int MusicSongsToolBoxWidget::count() const
{
    return m_itemList.count();
}

void MusicSongsToolBoxWidget::addItem(QWidget *item, const QString &text)
{
    int count = m_layout->count();
    if(count > 1)
    {
        m_layout->removeItem(m_layout->itemAt(count - 1));
    }

    //hide before widget
    for(int i=0; i<m_itemList.count(); ++i)
    {
        m_itemList[i]->setItemHide(false);
    }

    // Add item and make sure it stretches the remaining space.
    MusicSongsToolBoxWidgetItem *it = new MusicSongsToolBoxWidgetItem(m_itemList.count(), text, this);
    it->addItem(item);
    m_itemList.append(it);
    m_layout->addWidget(it);
    m_layout->addStretch(5);
}

void MusicSongsToolBoxWidget::removeItem(QWidget *item)
{
    for(int i=0; i<m_itemList.count(); ++i)
    {
        if(m_itemList[i]->item(0) == item)
        {
            m_layout->removeWidget(item);
            m_itemList.takeAt(i)->deleteLater();
            m_currentIndex = 0;
            break;
        }
    }
}

void MusicSongsToolBoxWidget::mousePressEvent(QMouseEvent *event)
{
    Q_UNUSED(event);
}

void MusicSongsToolBoxWidget::contextMenuEvent(QContextMenuEvent *event)
{
    Q_UNUSED(event);
}