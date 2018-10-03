#include <QTimer>
#include <QPainter>
#include <QPaintEvent>
#include <math.h>
#include <stdlib.h>
#include <qmmp/buffer.h>
#include <qmmp/output.h>
#include <qmmp/soundcore.h>
#include "fft.h"
#include "inlines.h"
#include "flashmeter.h"

#define DISTANCE    100

FlashMeter::FlashMeter (QWidget *parent) : Visual (parent)
{
    m_intern_vis_data = 0;
    m_x_scale = 0;
    m_running = false;
    m_rows = 0;
    m_cols = 0;

    setWindowTitle (tr("Flash Meter Widget"));
    m_timer = new QTimer (this);
    connect(m_timer, SIGNAL (timeout()), this, SLOT (timeout()));

    m_analyzer_falloff = 1.2;
    m_timer->setInterval(10);
    m_cell_size = QSize(6, 2);

    clear();
}

FlashMeter::~FlashMeter()
{
    if(m_intern_vis_data)
        delete [] m_intern_vis_data;
    if(m_x_scale)
        delete [] m_x_scale;
}

void FlashMeter::start()
{
    m_running = true;
    if(isVisible())
    {
        m_timer->start();
    }
}

void FlashMeter::stop()
{
    m_running = false;
    m_timer->stop();
    clear();
}

void FlashMeter::clear()
{
    m_rows = 0;
    m_cols = 0;
    update();
}

void FlashMeter::timeout()
{
    if(takeData(m_left_buffer, m_right_buffer))
    {
        process();
        update();
    }
}

void FlashMeter::hideEvent (QHideEvent *)
{
    m_timer->stop();
}

void FlashMeter::showEvent (QShowEvent *)
{
    if(m_running)
    {
        m_timer->start();
    }
}

void FlashMeter::paintEvent (QPaintEvent * e)
{
    QPainter painter (this);
    painter.fillRect(e->rect(), Qt::black);
    draw(&painter);
}

void FlashMeter::process ()
{
    static fft_state *state = 0;
    if (!state)
        state = fft_init();

    int rows = (height() - 2) / m_cell_size.height();
    int cols = (width() - 2) / m_cell_size.width();

    if(m_rows != rows || m_cols != cols)
    {
        m_rows = rows;
        m_cols = cols;
        if(m_intern_vis_data)
            delete [] m_intern_vis_data;
        if(m_x_scale)
            delete [] m_x_scale;
        m_intern_vis_data = new double[m_cols];
        m_x_scale = new int[m_cols + 1];

        for(int i = 0; i < m_cols; ++i)
        {
            m_intern_vis_data[i] = 0;
        }
        for(int i = 0; i < m_cols + 1; ++i)
            m_x_scale[i] = pow(pow(255.0, 1.0 / m_cols), i);
    }

    short dest[256];
    short y;
    int k, magnitude;

    calc_freq (dest, m_left_buffer);

    double y_scale = (double) 1.25 * m_rows / log(256);

    for (int i = 0; i < m_cols; i++)
    {
        y = 0;
        magnitude = 0;

        if(m_x_scale[i] == m_x_scale[i + 1])
        {
            y = dest[i];
        }
        for (k = m_x_scale[i]; k < m_x_scale[i + 1]; k++)
        {
            y = qMax(dest[k], y);
        }

        y >>= 7; //256

        if (y)
        {
            magnitude = int(log (y) * y_scale);
            magnitude = qBound(0, magnitude, m_rows);
        }

        m_intern_vis_data[i] -= m_analyzer_falloff * m_rows / 15;
        m_intern_vis_data[i] = magnitude > m_intern_vis_data[i] ? magnitude : m_intern_vis_data[i];
    }
}

void FlashMeter::draw (QPainter *p)
{
    if(m_cols == 0)
    {
        return;
    }

    p->drawPixmap(rect(), QPixmap(":/img/bg"));

    p->setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
    p->setPen(QPen(QColor(64, 229, 255), 3));
    p->translate(rect().center());

    qreal startAngle = 45;
    for (int i = 0; i < DISTANCE; ++i)
    {
        p->save();
        p->rotate(startAngle);
        QPointF bottomPot(0, DISTANCE);
        QPointF topPot(0, DISTANCE + m_intern_vis_data[int(i*m_cols*1.0/DISTANCE)]);
        p->drawLine(bottomPot, topPot);
        p->restore();
        startAngle += 3.6;
    }
}