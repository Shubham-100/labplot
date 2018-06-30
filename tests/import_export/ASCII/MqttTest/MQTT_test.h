#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QtMqtt/QMqttClient>
#include <QtMqtt/QMqttTopicName>
#include <QTimer>
#include <QVector>
#include <random>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

public slots:
    void setClientPort(int p);

private slots:
    void on_buttonConnect_clicked();
    void on_buttonQuit_clicked();    
    void brokerDisconnected();   
	void on_buttonSubscribe_clicked();
	void onTimeout();
	void intervalChanged(const QString&);
	void onConnect();

private:
    Ui::MainWindow *ui;
    QMqttClient *m_client;
	bool m_run;
	QMqttTopicName* m_brownianX;
	quint8 m_qos;
	QTimer* m_timer;
	unsigned m_seed;
	std::default_random_engine* m_generator;
	std::normal_distribution<double>* m_distribution;
	double m_delta;
	double m_dt;
	int m_pathes;
	int m_iters;
	int m_iterCount;
	int m_itersTotal;
	int m_interval;

	QVector<QMqttTopicName*> m_brownianTopics;
	QVector<double> m_x;
};

#endif // MAINWINDOW_H