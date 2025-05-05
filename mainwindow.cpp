#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QPainter>
#include <QKeyEvent>
#include <QtMath>
#include <QRandomGenerator>
#include <QRadialGradient>
#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
    ui(new Ui::MainWindow),
    hookAngle(0),
    hookLength(50),
    hookSpeed(0),
    stretching(false),
    retracting(false),
    score(0),
    timeRemaining(60),
    targetScore(1000),
    gameOver(false)
{
    ui->setupUi(this);
    setFixedSize(800, 600);

    timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &MainWindow::updateFrame);
    timer->start(16); // 游戏更新的帧率，约60FPS

    countdownTimer = new QTimer(this);
    connect(countdownTimer, &QTimer::timeout, [=]() {
        if (!gameOver) {
            timeRemaining--;
            if (timeRemaining <= 0) {
                endGame("时间已到，游戏结束！");
            }
            update();
        }
    });
    countdownTimer->start(1000); // 每秒更新时间

    initGame();
}

MainWindow::~MainWindow() {
    delete ui;
}

void MainWindow::initGame() {
    items.clear();
    for (int i = 0; i < 10; ++i) {
        Item item;
        item.pos = QPointF(QRandomGenerator::global()->bounded(50, 750),
                           QRandomGenerator::global()->bounded(300, 550));
        item.size = (QRandomGenerator::global()->bounded(2, 5)) * 10;  // 随机物品大小
        item.caught = false;
        item.moving = false;
        item.weight = item.size / 10; // 根据物品大小设定权重

        // 随机生成物品类型：金矿、宝石、炸弹
        int type = QRandomGenerator::global()->bounded(3);
        if (type == 0) {
            item.type = Gold;
        } else if (type == 1) {
            item.type = Gem;
        } else {
            item.type = Bomb;
        }

        items.append(item);
    }
}

void MainWindow::resetHook() {
    hookLength = 50;
    hookSpeed = 0;
    stretching = false;
    retracting = false;
}

void MainWindow::paintEvent(QPaintEvent *) {
    QPainter painter(this);

    // 背景 - 渐变模拟天空
    QLinearGradient bgGrad(0, 0, 0, height());
    bgGrad.setColorAt(0, QColor(135, 206, 235)); // 天空蓝
    bgGrad.setColorAt(1, QColor(210, 180, 140)); // 地面棕黄
    painter.fillRect(rect(), bgGrad);

    // 分数与倒计时显示
    painter.setPen(Qt::black);
    painter.setFont(QFont("Arial", 18));
    painter.drawText(10, 30, QString("Score: %1").arg(score));
    painter.drawText(10, 60, QString("Time: %1s").arg(timeRemaining));
    painter.drawText(10, 90, QString("Target: %1").arg(targetScore));

    // 钩子绘制
    QPointF start(400, 0);
    QPointF end(400 + hookLength * qSin(qDegreesToRadians(hookAngle)),
                hookLength * qCos(qDegreesToRadians(hookAngle)));
    painter.setPen(QPen(Qt::black, 3));
    painter.drawLine(start, end);

    // 绘制钩子头
    painter.setBrush(Qt::darkGray);
    painter.setPen(Qt::NoPen);
    painter.drawEllipse(end, 6, 6);

    // 绘制 Jett 人物
    int jettX = end.x() - 15;
    int jettY = end.y() - 35;
    painter.setBrush(QColor(255, 224, 189)); painter.drawRect(jettX, jettY, 20, 20);  // 头
    painter.setBrush(QColor(30, 144, 255)); painter.drawRect(jettX - 5, jettY - 5, 30, 10); // 发
    painter.setBrush(Qt::black); painter.drawEllipse(jettX + 5, jettY + 5, 3, 3); painter.drawEllipse(jettX + 12, jettY + 5, 3, 3); // 眼
    painter.setBrush(QColor(255, 182, 193)); painter.drawRect(jettX + 7, jettY + 15, 6, 2); // 嘴
    painter.setBrush(QColor(0, 128, 128)); painter.drawRect(jettX, jettY + 20, 20, 20); // 衣
    painter.setBrush(QColor(0, 64, 64)); painter.drawRect(jettX, jettY + 40, 20, 15); // 裤
    painter.setBrush(QColor(105, 105, 105)); painter.drawRect(jettX, jettY + 55, 20, 5); // 靴
    painter.setPen(QPen(Qt::black, 3));
    painter.drawLine(jettX + 3, jettY + 25, jettX - 10, jettY + 35); // 左手
    painter.drawLine(jettX + 17, jettY + 25, jettX + 30, jettY + 35); // 右手

    // 绘制金矿、宝石和炸弹
    for (const Item &item : items) {
        if (!item.caught) {
            if (item.type == Gold) {
                // 绘制金矿
                QRadialGradient gradient(item.pos, item.size);
                gradient.setColorAt(0.0, QColor(255, 215, 0)); // 金矿颜色
                gradient.setColorAt(1.0, QColor(184, 134, 11)); // 深色边缘
                painter.setBrush(gradient);
                painter.setPen(Qt::NoPen);
                painter.drawEllipse(item.pos, item.size, item.size);
            } else if (item.type == Gem) {
                // 绘制宝石
                QRadialGradient gemGradient(item.pos, item.size * 0.7);
                gemGradient.setColorAt(0.0, QColor(255, 0, 255)); // 宝石中央紫色
                gemGradient.setColorAt(1.0, QColor(128, 0, 128)); // 深紫色边缘
                painter.setBrush(gemGradient);
                painter.setPen(Qt::NoPen);
                painter.drawEllipse(item.pos, item.size, item.size);
            } else if (item.type == Bomb) {
                // 绘制炸弹
                QRadialGradient bombGradient(item.pos, item.size * 0.8);
                bombGradient.setColorAt(0.0, QColor(255, 0, 0)); // 炸弹中央红色
                bombGradient.setColorAt(1.0, QColor(139, 0, 0)); // 深红色边缘
                painter.setBrush(bombGradient);
                painter.setPen(Qt::NoPen);
                painter.drawEllipse(item.pos, item.size, item.size);
            }
        }
    }
}

void MainWindow::keyPressEvent(QKeyEvent *event) {
    if (event->key() == Qt::Key_Space && !stretching && !retracting && !gameOver) {
        stretching = true;
        hookSpeed = 5;
    }
}

void MainWindow::updateFrame() {
    if (gameOver) return;

    // 控制钩子的伸缩
    if (!stretching && !retracting) {
        static bool rotateRight = true;
        hookAngle += rotateRight ? 1 : -1;
        if (hookAngle > 75) rotateRight = false;
        if (hookAngle < -75) rotateRight = true;
    } else {
        hookLength += hookSpeed;

        if (stretching && hookLength > 600) {
            stretching = false;
            retracting = true;
            hookSpeed = -5;
        }

        if (retracting && hookLength <= 50) {
            for (Item &item : items) {
                if (item.moving) {
                    item.caught = true;
                    item.moving = false;
                    // 根据物品类型调整分数
                    if (item.type == Gold) {
                        score += item.size * 10;
                    } else if (item.type == Gem) {
                        score += item.size * 20;  // 宝石增加更多分数
                    } else if (item.type == Bomb) {
                        score -= item.size * 15;  // 炸弹减少分数
                    }
                }
            }
            resetHook();
        }

        if (stretching) {
            checkCatch();
        }
    }

    QPointF hookPos(400 + hookLength * qSin(qDegreesToRadians(hookAngle)),
                    hookLength * qCos(qDegreesToRadians(hookAngle)));
    for (Item &item : items) {
        if (item.moving) {
            item.pos = hookPos;
        }
    }

    if (score >= targetScore) {
        endGame(QString("恭喜你达成目标分数 %1！").arg(targetScore));
    }

    update();
}

void MainWindow::checkCatch() {
    QPointF hookPos(400 + hookLength * qSin(qDegreesToRadians(hookAngle)),
                    hookLength * qCos(qDegreesToRadians(hookAngle)));

    for (Item &item : items) {
        if (!item.caught) {
            double dist = QLineF(hookPos, item.pos).length();
            if (dist <= item.size + 5) {
                item.moving = true;
                retracting = true;
                stretching = false;
                hookSpeed = -qMax(1.0, 6.0 / item.weight);
                break;
            }
        }
    }
}

void MainWindow::endGame(const QString &message) {
    gameOver = true;
    timer->stop();
    countdownTimer->stop();

    QMessageBox::StandardButton reply = QMessageBox::question(
        this, "游戏结束", message + "\n是否重新开始？",
        QMessageBox::Yes | QMessageBox::No);

    if (reply == QMessageBox::Yes) {
        initGame();
        resetHook();
        score = 0;
        timeRemaining = 60;
        gameOver = false;
        timer->start(16);
        countdownTimer->start(1000);
    }
}
