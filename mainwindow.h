#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "socketconnect.h"
#include <QLineEdit>
#include <QPushButton>
#include <QSignalMapper>
#include <QMap>
#include <QHash>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private:
    Ui::MainWindow *ui;
    SocketConnect *socket;
    int m_mode = 0;
    QSignalMapper *m_buttonMapper;
    QMap<int, QLineEdit *> m_lineeditList;
    QMap<int, QString> m_currencyList;
    QWidget *m_walletWidget;
    double m_maxTrade;
    int m_eventNumber = -1;

    double simulatedCurrency = 1;
    double simulatedCash = 0;
    double estimatedValue = 0;

    QList<QPair<double, double>> traderList;
    Double tradeLine = -1;

private slots:
    void onConnect();
    void parseMessages(QByteArray message);
    void startTrader(int id);
};

#endif // MAINWINDOW_H
