/**
 * @file GameController.h
 * @brief 游戏控制器
 * @author [pengchengkang]
 * @date 2025.06.17
 */

#ifndef GAMECONTROLLER_H
#define GAMECONTROLLER_H

#include <QObject>

namespace Fantasy {

/**
 * @brief 游戏控制器类
 * 
 * 负责控制游戏逻辑和状态
 */
class GameController : public QObject
{
    Q_OBJECT

public:
    explicit GameController(QObject* parent = nullptr);
    ~GameController();

    // 游戏控制
    void update();
    void startGame();
    void pauseGame();
    void resumeGame();
    void stopGame();

signals:
    void gameStateChanged(const QString& state);

private:
    bool m_isRunning;  ///< 游戏是否运行中
};

} // namespace Fantasy

#endif // GAMECONTROLLER_H 