#include <QTableWidget>
#include <QHeaderView>
#include <QPushButton>
#include <QPainter>
#include <QVBoxLayout>
#include <QDebug>
#include "IconHelper.h"
#include "DateEdit.h"

#define DATE_FORMAT_DAY "yyyy-MM-dd"
#define DATE_FORMAT_MONTH "yyyy-MM"
#define DATE_FORMAT_YEAR "yyyy"

static int GetDateFromText(const QString& dateText, QDate& dateStart, QDate& dateEnd)
{
    auto dateList = dateText.split(QChar('~'));
    int dateCnt = 0;
    if (dateList.empty())
    {
        dateCnt = 0;
    }
    else if (dateList.size() == 1)
    {
        dateStart = QDate::fromString(dateList[0], DATE_FORMAT_DAY);
        if (dateStart.isValid())
            dateCnt ++;
    }
    else
    {
        dateStart = QDate::fromString(dateList[0], DATE_FORMAT_DAY);
        dateEnd   = QDate::fromString(dateList[1], DATE_FORMAT_DAY);
        if (dateStart.isValid())
            dateCnt ++;
        if(dateEnd.isValid())
            dateCnt ++;
    }
    return dateCnt;
}

int DateEdit::GetDate(DateEdit* dateEdit, QDate& dateStart, QDate& dateEnd)
{
    QString dateText = dateEdit->text();
    return ::GetDateFromText(dateText, dateStart, dateEnd);
}

DateEdit::DateEdit(QWidget* parent)
    : QLineEdit(parent)
{
    m_btnPopup = new QPushButton(this);
    m_btnPopup->setObjectName("btnPopup");
    m_btnPopup->resize(25, 25);
    m_btnPopup->setIconSize(QSize(22, 24));
    m_btnPopup->setStyleSheet("background-color:transparent;");
    IconHelper::Instance()->SetIcon(m_btnPopup, QChar(0xf073), 10);

    m_btnAdd = new QPushButton(this);
    m_btnAdd->setFixedSize(20, 14);
    m_btnAdd->setIconSize(QSize(20, 14));
    m_btnAdd->setStyleSheet("background-color:transparent;");
    IconHelper::Instance()->SetIcon(m_btnAdd, QChar(0xf106), 10);

    m_btnSub = new QPushButton(this);
    m_btnSub->setFixedSize(20, 14);
    m_btnSub->setIconSize(QSize(20, 14));
    m_btnSub->setStyleSheet("background-color:transparent;");
    IconHelper::Instance()->SetIcon(m_btnSub, QChar(0xf107), 10);
    m_editMode = DateEditMode::Total;

    m_date = QDate::currentDate();
    installEventFilter(this);

    for (int i = (int)DateEditMode::Day; i < (int)DateEditMode::Total; i ++)
    {
        auto mode = static_cast<DateEditMode>(i);
        auto calendarWnd = ICalendarFactory::BuildICalendarWnd(mode, this);
        if (calendarWnd)
        {
            calendarWnd->raise();
            calendarWnd->hide();
            connect(calendarWnd, &ICalendarWnd::CalendarChanged, this, &DateEdit::OnCalendarChanged);
        }
        m_calendarWndTbl[i] = calendarWnd;
    }
    this->setFocusPolicy(Qt::NoFocus);
    this->SetEditMode(DateEditMode::Day);
    connect(m_btnPopup, &QPushButton::clicked, this, &DateEdit::OnBtnPopupClicked);
    connect(m_btnAdd  , &QPushButton::clicked, this, &DateEdit::OnBtnAddOrSubClicked);
    connect(m_btnSub  , &QPushButton::clicked, this, &DateEdit::OnBtnAddOrSubClicked);
}

DateEdit::~DateEdit()
{}

void DateEdit::SetEditMode(DateEditMode mode)
{
    if (m_editMode == mode) return;
    m_editMode = mode;
    auto calendarWnd = m_calendarWndTbl[(int)m_editMode];
    if (calendarWnd)
    {
        calendarWnd->SetDateEditText(this);
        calendarWnd->raise();
        SetBtnSymbol(ButtonSymbol::Calendar);
    }
    else
    {
        this->setText(m_date.toString(DATE_FORMAT_YEAR));
        SetBtnSymbol(ButtonSymbol::Arrow);
    }
}

void DateEdit::SetDate(const QDate& date)
{
    m_date = date;
}

QDate DateEdit::GetDate() const
{
    return m_date;
}

void DateEdit::OnBtnPopupClicked()
{
    auto calendarWnd = m_calendarWndTbl[(int)m_editMode];
    if (calendarWnd->isHidden())
    {
        calendarWnd->SetCalendarDate(this->text(), m_date);
        calendarWnd->move(this->mapToGlobal(QPoint(1, this->rect().bottom() + 1)));
        calendarWnd->raise();
        calendarWnd->show();
        calendarWnd->activateWindow();
    }
    else
    {
        calendarWnd->hide();
    }
}

void DateEdit::OnCalendarChanged()
{
    QString dateText;
    auto calendarWnd = m_calendarWndTbl[(int)m_editMode];
    calendarWnd->GetCalendarDate(dateText, m_date);
    calendarWnd->hide();
    this->setText(dateText);
}

void DateEdit::SetBtnSymbol(DateEdit::ButtonSymbol symbol)
{
    if (symbol == DateEdit::Calendar)
    {
        m_btnPopup->setVisible(true);
        m_btnAdd->setVisible(false);
        m_btnSub->setVisible(false);
    }
    else if (symbol == DateEdit::Arrow)
    {
        m_btnPopup->setVisible(false);
        m_btnAdd->setVisible(true);
        m_btnSub->setVisible(true);
    }
    else
    {
        NULL;
    }
}

void DateEdit::OnBtnAddOrSubClicked()
{
    auto btn = qobject_cast<QPushButton*>(sender());
    int year = m_date.year();
    int day  = m_date.day();
    if (QDate::isLeapYear(year) && m_date.month() == 2 && m_date.day() == 29)
    {
        day -= 1;
    }

    if (btn == m_btnAdd)
    {
        m_date.setDate(year+1, m_date.month(), day);
    }
    else if (btn == m_btnSub)
    {
        m_date.setDate(year-1, m_date.month(), day);
    }
    else
    {
        NULL;
    }
    this->setText(m_date.toString(DATE_FORMAT_YEAR));
}

void DateEdit::resizeEvent(QResizeEvent* event)
{
    QLineEdit::resizeEvent(event);
    int h = height();
    m_btnPopup->move(width() - m_btnPopup->width(), (h - m_btnPopup->height())/2);
    m_btnAdd->move(width() - m_btnAdd->width() - 1, 0);
    m_btnSub->move(width() - m_btnSub->width() - 1, h - m_btnSub->height());
}

bool DateEdit::eventFilter(QObject* obj, QEvent* event)
{
    // 不支持手动修改日期，需要在日历控件中做出选择
    if (event->type() == QEvent::KeyPress || event->type() == QEvent::Wheel)
    {
        return true;
    }
    return QLineEdit::eventFilter(obj, event);
}

CustomCalendar::CustomCalendar(QWidget* parent)
    : QCalendarWidget(parent)
{
    // 初始化日历控件，隐藏不必要的元素
    setGridVisible(false);
    setNavigationBarVisible(true);
    setVerticalHeaderFormat(QCalendarWidget::NoVerticalHeader);
    setHorizontalHeaderFormat(QCalendarWidget::ShortDayNames);
    this->setFocusPolicy(Qt::StrongFocus);
    this->setFirstDayOfWeek(Qt::Monday);

    this->setObjectName("customcalendar");
    m_date = QDate::currentDate();
    weekMask = false;

    connect(this, &CustomCalendar::clicked, this, &CustomCalendar::SetCurDate);
}

CustomCalendar::~CustomCalendar()
{}

void CustomCalendar::SetWeekMask(bool enable)
{
    weekMask = enable;
}

bool CustomCalendar::GetWeekMask() const
{
    return weekMask;
}

void CustomCalendar::SetCurDate(const QDate& date)
{
    m_date = date;
    this->setSelectedDate(m_date);
    this->update();
}

QDate CustomCalendar::GetCurDate() const
{
    return m_date;
}

// 重写鼠标点击事件处理函数，以仅选择年和月
void CustomCalendar::mousePressEvent(QMouseEvent* event)
{
#if 0
    QDate date = dateAt(event->pos());
    if (date.isValid()) {
        // 更新当前日期，但不显示具体日期
        setCurrentPage(date.year(), date.month());
        // 发出信号，通知外部选中的年份和月份
        emit yearMonthSelected(date.year(), date.month());
    }
#endif
    QCalendarWidget::mousePressEvent(event);
}

bool CustomCalendar::isSameWeek(const QDate& date1, const QDate& date2) const
{
    qint64 daysDiff = qAbs(date1.daysTo(date2));
    if (daysDiff >= 7) return false;
    int dayOfWeek1 = date1.dayOfWeek();
    int dayOfWeek2 = date2.dayOfWeek();
    return qAbs(dayOfWeek1 - dayOfWeek2) == daysDiff;
}

// 重写绘制单元格的方法，隐藏日期数字
void CustomCalendar::paintCell(QPainter* painter, const QRect& rect, const QDate& date) const
{
    painter->save();
    if (m_date == date)
    {
        painter->setPen(Qt::red);
        painter->setBrush(QColor(0, 145, 255));
        painter->drawRect(rect.x(), rect.y(), rect.width()-1, rect.height()-1);
        painter->setPen(QColor(255, 255, 255));
        QRect dateRect = QRect(rect.x() + 3, rect.y() + 3, rect.width() - 6, rect.height() - 6);
        painter->drawText(dateRect, Qt::AlignCenter, QString::number(date.day()));
    }
    else if (weekMask && isSameWeek(m_date, date))
    {
        painter->setPen(Qt::NoPen);
        painter->setBrush(QColor(0, 145, 255));
        painter->drawRoundedRect(rect.x() + 1, rect.y() + 1, rect.width() - 2, rect.height() - 2, 3, 3);
        painter->setPen(QColor(255, 255, 255));
        QRect dateRect = QRect(rect.x() + 3, rect.y() + 3, rect.width() - 6, rect.height() - 6);
        painter->drawText(dateRect, Qt::AlignCenter, QString::number(date.day()));
    }
    else
    {
        QCalendarWidget::paintCell(painter, rect, date);
    }
    painter->restore();
}


ICalendarWnd::ICalendarWnd(QWidget* parent)
    : QWidget(parent)
{
    this->setWindowFlags(Qt::FramelessWindowHint | Qt::Popup);
}

ICalendarWnd::~ICalendarWnd()
{}

void ICalendarWnd::paintEvent(QPaintEvent* e)
{
    QStyleOption opt;
    opt.init(this);
    QPainter p(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
    QWidget::paintEvent(e);
}

//////////////////////////////// DayCalendar  ///////////////////////////////////////
DayCalendarWnd::DayCalendarWnd(QWidget* parent)
    : ICalendarWnd(parent)

{
    QHBoxLayout* hLayout = new QHBoxLayout(this);
    calendar = new CustomCalendar(this);
    hLayout->addWidget(calendar);
    connect(calendar, &QCalendarWidget::clicked, this, &DayCalendarWnd::CalendarChanged);
}

DayCalendarWnd::~DayCalendarWnd()
{}

CustomCalendar* DayCalendarWnd::GetCalendar()
{
    return calendar;
}

void DayCalendarWnd::SetCalendarDate(const QString& dateText, const QDate& date)
{
    Q_UNUSED(date);
    QDate dateShow = QDate::fromString(dateText, DATE_FORMAT_DAY);
    if (dateShow.isValid())
    {
        calendar->SetCurDate(dateShow);
        calendar->setCurrentPage(dateShow.year(), dateShow.month());
    }
}

void DayCalendarWnd::GetCalendarDate(QString& dateText, QDate& date)
{
    date = calendar->GetCurDate();
    dateText = date.toString(DATE_FORMAT_DAY);
}

void DayCalendarWnd::SetDateEditText(DateEdit *dateEdit)
{
    QDate date = dateEdit->GetDate();
    dateEdit->setText(date.toString(DATE_FORMAT_DAY));
}

//////////////////////////////// WeekCalendarWnd ///////////////////////////////////////
WeekCalendarWnd::WeekCalendarWnd(QWidget* parent)
    : ICalendarWnd(parent)

{
    QHBoxLayout* hLayout = new QHBoxLayout(this);
    calendar = new CustomCalendar(this);
    hLayout->addWidget(calendar);
    connect(calendar, &QCalendarWidget::clicked, this, &WeekCalendarWnd::CalendarChanged);
    calendar->SetWeekMask(true);
}

WeekCalendarWnd::~WeekCalendarWnd()
{}

CustomCalendar* WeekCalendarWnd::GetCalendar()
{
    return calendar;
}

void WeekCalendarWnd::SetCalendarDate(const QString& dateText, const QDate& date)
{
    Q_UNUSED(dateText);
    calendar->SetCurDate(date);
    calendar->setCurrentPage(date.year(), date.month());
}

void WeekCalendarWnd::GetCalendarDate(QString& dateText, QDate& date)
{
    date = calendar->GetCurDate();
    int dayOfWeek = date.dayOfWeek();
    int mondayOffset = dayOfWeek - 1;
    int sundayOffset = 7 - dayOfWeek;
    QDate monday = date.addDays(-mondayOffset);
    QDate sunday = date.addDays(sundayOffset);
    dateText = QString("%1~%2").arg(monday.toString(DATE_FORMAT_DAY))
                               .arg(sunday.toString(DATE_FORMAT_DAY));
}

void WeekCalendarWnd::SetDateEditText(DateEdit *dateEdit)
{
    QDate date = dateEdit->GetDate();
    QDate monday = date.addDays(1 - date.dayOfWeek());
    QDate sunday = date.addDays(7 - date.dayOfWeek());
    dateEdit->setText(QString("%1~%2").arg(monday.toString(DATE_FORMAT_DAY))
                                      .arg(sunday.toString(DATE_FORMAT_DAY)));
}

//////////////////////////////// DoubleDayCalendarWnd ///////////////////////////////////////
DoubleDayCalendarWnd::DoubleDayCalendarWnd(QWidget* parent)
    : ICalendarWnd(parent)
{
    QHBoxLayout* hLyout1 = new QHBoxLayout();
    m_calendarStart = new CustomCalendar(this);
    m_calendarEnd = new CustomCalendar(this);
    hLyout1->addWidget(m_calendarStart);
    hLyout1->addWidget(m_calendarEnd);
    hLyout1->setSpacing(1);

    QHBoxLayout* hLyout2 = new QHBoxLayout();
    QSpacerItem* item = new QSpacerItem(10, 10, QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_labTime = new QLabel(this);
    m_btnOK = new QPushButton(this);
    m_labTime->setFixedSize(120, 25);
    m_btnOK->setFixedSize(50, 25);
    hLyout2->addWidget(m_labTime);
    hLyout2->addSpacerItem(item);
    hLyout2->addWidget(m_btnOK);
    m_btnOK->setText("OK");
    m_btnOK->setStyleSheet("");

    QVBoxLayout* vLayout = new QVBoxLayout(this);
    vLayout->addItem(hLyout1);
    vLayout->addItem(hLyout2);
    vLayout->setStretch(0, 1);

    connect(m_btnOK, &QPushButton::clicked, this, &DoubleDayCalendarWnd::OnBtnClicked);
}

DoubleDayCalendarWnd::~DoubleDayCalendarWnd()
{}

CustomCalendar* DoubleDayCalendarWnd::GetCalendar()
{
    return m_calendarStart;
}

void DoubleDayCalendarWnd::SetCalendarDate(const QString& dateText, const QDate& date)
{
    QDate dateStart, dateEnd;
    int cnt = ::GetDateFromText(dateText, dateStart, dateEnd);
    if (cnt == 2)
    {
        m_calendarStart->SetCurDate(dateStart);
        m_calendarEnd->SetCurDate(dateEnd);
        m_calendarStart->setCurrentPage(dateStart.year(), dateStart.month());
        m_calendarEnd->setCurrentPage(dateEnd.year(), dateEnd.month());
    }
}

void DoubleDayCalendarWnd::GetCalendarDate(QString& dateText, QDate& date)
{
    QString text1 = m_calendarStart->GetCurDate().toString(DATE_FORMAT_DAY);
    QString text2 = m_calendarEnd->GetCurDate().toString(DATE_FORMAT_DAY);
    dateText = QString("%1~%2").arg(text1).arg(text2);
    date = m_calendarStart->GetCurDate();
}

void DoubleDayCalendarWnd::SetDateEditText(DateEdit *dateEdit)
{
    QDate date = dateEdit->GetDate();
    dateEdit->setText(QString("%1~%2").arg(date.toString(DATE_FORMAT_DAY))
                                      .arg(date.toString(DATE_FORMAT_DAY)));
}

void DoubleDayCalendarWnd::OnBtnClicked()
{
    QDate dateStart = m_calendarStart->GetCurDate();
    QDate dateEnd   = m_calendarEnd->GetCurDate();

    if (dateStart > dateEnd)
    {
        return;
    }

    emit this->CalendarChanged();
}

void DoubleDayCalendarWnd::showEvent(QShowEvent*)
{
    m_labTime->setText(QString("today:%1").arg(QDate::currentDate().toString(DATE_FORMAT_DAY)));
}

//////////////////////////////// YearMonthCalendarWnd ///////////////////////////////////////
YearMonthCalendarWnd::YearMonthCalendarWnd(QWidget* parent)
    : ICalendarWnd(parent)
{
    QHBoxLayout* lay1 = new QHBoxLayout();
    QPushButton* m_btnPrv = new QPushButton(this);
    QPushButton* m_btnNxt = new QPushButton(this);
    m_label = new QLabel(this);

    m_btnPrv->setFixedSize(30, 30);
    m_btnNxt->setFixedSize(30, 30);
    m_label->setFixedHeight(30);
    m_label->setAlignment(Qt::AlignCenter);

    m_btnPrv->setText("<");
    m_btnNxt->setText(">");

    lay1->addWidget(m_btnPrv);
    lay1->addWidget(m_label);
    lay1->addWidget(m_btnNxt);

    QTableWidget* tableWidget = new QTableWidget(this);
    tableWidget->setRowCount(2);
    tableWidget->setColumnCount(6);

    QStringList months;
    months << "January" << "February" << "March" << "April" << "May" << "June" << "July"
        << "August" << "September" << "October" << "November" << "December";

    for (int row = 0; row < 2; row++)
    {
        for (int col = 0; col < 6; col++)
        {
            QTableWidgetItem* item = new QTableWidgetItem(months[row * 6 + col]);
            tableWidget->setItem(row, col, item);
        }
    }
    // 隐藏表头
    tableWidget->horizontalHeader()->hide();
    tableWidget->verticalHeader()->hide();
    tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    tableWidget->verticalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    QVBoxLayout* lay = new QVBoxLayout(this);
    lay->addItem(lay1);
    lay->addWidget(tableWidget);

    this->setFixedSize(350, 145);

    connect(m_btnPrv, &QPushButton::clicked, [=] { emit this->YearChanged(true); });
    connect(m_btnNxt, &QPushButton::clicked, [=] { emit this->YearChanged(false); });
    connect(this, &YearMonthCalendarWnd::YearChanged, this, &YearMonthCalendarWnd::OnYearChanged);
    connect(tableWidget, &QTableWidget::cellClicked, this, &YearMonthCalendarWnd::OnCellClicked);

    m_year = 2025;
    m_month = 1;
    m_label->setText(QString("%1Year %2Month").arg(m_year).arg(m_month));
}

YearMonthCalendarWnd::~YearMonthCalendarWnd()
{}

void YearMonthCalendarWnd::SetCalendarDate(const QString& dateText, const QDate& date)
{
    Q_UNUSED(date);
    QDate dateShow = QDate::fromString(dateText, DATE_FORMAT_MONTH);
    if (dateShow.isValid())
    {
        m_year  = dateShow.year();
        m_month = dateShow.month();
        m_label->setText(QString("%1Year %2Month").arg(m_year).arg(m_month));
    }
}

void YearMonthCalendarWnd::GetCalendarDate(QString& dateText, QDate& date)
{
    date.setDate(m_year, m_month, 1);
    dateText = date.toString(DATE_FORMAT_MONTH);
}

void YearMonthCalendarWnd::SetDateEditText(DateEdit *dateEdit)
{
    QDate date = dateEdit->GetDate();
    dateEdit->setText(date.toString(DATE_FORMAT_MONTH));
}

void YearMonthCalendarWnd::OnCellClicked(int row, int col)
{
    m_month = row * 6 + col + 1;
    m_label->setText(QString("%1Year %2Month").arg(m_year).arg(m_month));
    emit this->CalendarChanged();
}

void YearMonthCalendarWnd::OnYearChanged(bool pre)
{
    if (pre)
        m_year--;
    else
        m_year++;
    m_label->setText(QString("%1Year %2Month").arg(m_year).arg(m_month));
}

ICalendarWnd* ICalendarFactory::BuildICalendarWnd(DateEditMode mode, QWidget* parent)
{
    ICalendarWnd* calendar = nullptr;
    switch (mode)
    {
        case DateEditMode::Day:
        {
            calendar = new DayCalendarWnd(parent);
            calendar->setFixedSize(245, 265);
        }
        break;
        case DateEditMode::Week:
        {
            calendar = new WeekCalendarWnd(parent);
            calendar->setFixedSize(245, 265);
        }
        break;
        case DateEditMode::YearMonth:
        {
            calendar = new YearMonthCalendarWnd(parent);
            calendar->setFixedSize(245, 145);
        }
        break;
        case DateEditMode::Year:
        {
            calendar  = nullptr;
        }
        break;
        case DateEditMode::DoubleDay:
        {
            calendar = new DoubleDayCalendarWnd(parent);
            calendar->setFixedSize(475, 245);
        }
        break;
    default:
        break;
    }
    return calendar;
}
