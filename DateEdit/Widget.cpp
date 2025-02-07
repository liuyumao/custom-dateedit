#include <QVBoxLayout>
#include <QComboBox>
#include "DateEdit.h"
#include "Widget.h"
#include "ui_Widget.h"

Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
{
    ui->setupUi(this);
    QComboBox* comboBox = new QComboBox(this);
    DateEdit * dateEdit = new DateEdit(this);

    comboBox->addItem("Day");
    comboBox->addItem("Week");
    comboBox->addItem("YearMonth");
    comboBox->addItem("Year");
    comboBox->addItem("DoubleDay");

    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->addWidget(comboBox);
    layout->addWidget(dateEdit);
    layout->setContentsMargins(15, 15, 15, 15);

    this->setFixedSize(260, 160);

    connect(comboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [=](int i){
       auto mode = static_cast<DateEditMode>(i);
       dateEdit->SetEditMode(mode);
    });
}

Widget::~Widget()
{
    delete ui;
}

