#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>
#include <QVector>
#include <QPointF>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

enum ItemType { Gold, Gem, Bomb };  // 物品类型：金矿、宝石、炸弹

struct Item {
    QPointF pos;
    int size;
    bool caught;
    int weight;
    bool moving;
    ItemType type;  // 物品类型
};

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    void paintEvent(QPaintEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;

private slots:
    void updateFrame();

private:
    Ui::MainWindow *ui;
    QTimer *timer;
    QTimer *countdownTimer;
    QVector<Item> items;

    double hookAngle;
    double hookLength;
    double hookSpeed;
    bool stretching;
    bool retracting;
    int score;

    int timeRemaining;
    int targetScore;
    bool gameOver;

    void initGame();
    void resetHook();
    void checkCatch();
    void endGame(const QString &message);
};
#endif // MAINWINDOW_H
