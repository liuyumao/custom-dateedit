#ifndef DATE_EDIT_H
#define DATE_EDIT_H

#include <QWidget>
#include <QCalendarWidget>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QHash>
#include <QDate>
#include <QPainter>
#include <QMouseEvent>

enum class DateEditMode
{
    Day = 0,
    Week,
    YearMonth,
    Year,
    DoubleDay,
    Total
};

class ICalendarWnd;
class DateEdit : public QLineEdit
{
    Q_OBJECT

    using CalendarWndTable = QHash<int, ICalendarWnd*>;

public:
    enum ButtonSymbol
    {
        Arrow = 0,
        Calendar
    };
public:
    static int GetDate(DateEdit* DateEdit, QDate& dateStart, QDate& dateEnd);
public:
    DateEdit(QWidget* parent = Q_NULLPTR);
    ~DateEdit();
    void SetEditMode(DateEditMode mode);
    void SetDate(const QDate& date);
    QDate GetDate() const;
public slots:
    void OnBtnPopupClicked();
    void OnCalendarChanged();
    void SetBtnSymbol(DateEdit::ButtonSymbol symbol);
    void OnBtnAddOrSubClicked();

protected:
    virtual void resizeEvent(QResizeEvent* event) Q_DECL_OVERRIDE;
    virtual bool eventFilter(QObject* obj, QEvent* event) Q_DECL_OVERRIDE;
private:
    QPushButton *    m_btnPopup;
    QPushButton *    m_btnAdd;
    QPushButton *    m_btnSub;
    QDate            m_date;
    DateEditMode     m_editMode;
    CalendarWndTable m_calendarWndTbl;
};

class CustomCalendar : public QCalendarWidget
{
    Q_OBJECT
public:
    explicit CustomCalendar(QWidget* parent = Q_NULLPTR);
    ~CustomCalendar();
    void SetWeekMask(bool enable);
    bool GetWeekMask() const;
    void SetCurDate(const QDate& date);
    QDate GetCurDate() const;
protected:
    virtual void paintCell(QPainter* painter, const QRect& rect, const QDate& date) const Q_DECL_OVERRIDE;
    virtual void mousePressEvent(QMouseEvent* event) Q_DECL_OVERRIDE;

protected:
    bool isSameWeek(const QDate& date1, const QDate& date2) const;

private:
    QDate  m_date;
    bool   weekMask;
};

class ICalendarWnd : public QWidget
{
    Q_OBJECT
public:
    ICalendarWnd(QWidget* parent = Q_NULLPTR);
    virtual ~ICalendarWnd();
public:
    virtual CustomCalendar* GetCalendar() { return Q_NULLPTR; }
    virtual void SetCalendarDate(const QString&, const QDate&) {}
    virtual void GetCalendarDate(QString&, QDate&) {}
    virtual void SetDateEditText(DateEdit*) {}
protected:
    virtual void paintEvent(QPaintEvent*) Q_DECL_OVERRIDE;
signals:
    void CalendarChanged();
};

class DayCalendarWnd : public ICalendarWnd
{
    Q_OBJECT

public:
    DayCalendarWnd(QWidget* parent = Q_NULLPTR);
    virtual ~DayCalendarWnd() Q_DECL_OVERRIDE;
    virtual CustomCalendar* GetCalendar() Q_DECL_OVERRIDE;
    virtual void SetCalendarDate(const QString& dateText, const QDate& date) Q_DECL_OVERRIDE;
    virtual void GetCalendarDate(QString& dateText, QDate& date) Q_DECL_OVERRIDE;
    virtual void SetDateEditText(DateEdit* dateEdit) Q_DECL_OVERRIDE;
private:
    CustomCalendar* calendar;
};

class WeekCalendarWnd : public ICalendarWnd
{
    Q_OBJECT

public:
    WeekCalendarWnd(QWidget* parent = Q_NULLPTR);
    virtual ~WeekCalendarWnd() Q_DECL_OVERRIDE;
    virtual CustomCalendar* GetCalendar() Q_DECL_OVERRIDE;
    virtual void SetCalendarDate(const QString& dateText, const QDate& date) Q_DECL_OVERRIDE;
    virtual void GetCalendarDate(QString& dateText, QDate&) Q_DECL_OVERRIDE;
    virtual void SetDateEditText(DateEdit* dateEdit) Q_DECL_OVERRIDE;
private:
    CustomCalendar* calendar;
};

class DoubleDayCalendarWnd : public ICalendarWnd
{
    Q_OBJECT

public:
    DoubleDayCalendarWnd(QWidget* parent = Q_NULLPTR);
    virtual ~DoubleDayCalendarWnd() Q_DECL_OVERRIDE;
    virtual CustomCalendar* GetCalendar() Q_DECL_OVERRIDE;
    virtual void SetCalendarDate(const QString& dateText, const QDate& date) Q_DECL_OVERRIDE;
    virtual void GetCalendarDate(QString& dateText, QDate& date) Q_DECL_OVERRIDE;
    virtual void SetDateEditText(DateEdit* dateEdit) Q_DECL_OVERRIDE;
public slots:
    void OnBtnClicked();
protected:
    virtual void showEvent(QShowEvent* e) Q_DECL_OVERRIDE;
private:
    CustomCalendar*  m_calendarStart;
    CustomCalendar*  m_calendarEnd;
    QLabel     *  m_labTime;
    QPushButton*  m_btnOK;
};

class YearMonthCalendarWnd : public ICalendarWnd
{
    Q_OBJECT

public:
    explicit YearMonthCalendarWnd(QWidget* parent = Q_NULLPTR);
    virtual ~YearMonthCalendarWnd() Q_DECL_OVERRIDE;
    virtual void SetCalendarDate(const QString& dateText, const QDate& date) Q_DECL_OVERRIDE;
    virtual void GetCalendarDate(QString& dateText, QDate& date) Q_DECL_OVERRIDE;
    virtual void SetDateEditText(DateEdit* dateEdit) Q_DECL_OVERRIDE;
public slots:
    void OnCellClicked(int row, int col);
    void OnYearChanged(bool pre);
signals:
    void YearChanged(bool pre);
private:
    int       m_year;
    int       m_month;
    QLabel* m_label;
};

class ICalendarFactory
{
public:
    static ICalendarWnd* BuildICalendarWnd(DateEditMode mode, QWidget* parent = Q_NULLPTR);
    ICalendarFactory() = default;
    ~ICalendarFactory() = default;
};

#endif // DATE_EDIT_H
