/**
 * @file MainMenu.h
 * @brief 主菜单组件
 * @author [pengchengkang]
 * @date 2025.06.17
 */

#ifndef MAINMENU_H
#define MAINMENU_H

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QGraphicsDropShadowEffect>
#include <QPropertyAnimation>
#include <QTimer>

namespace Fantasy {

/**
 * @brief 主菜单组件
 * 
 * 提供游戏的主菜单界面，包括：
 * - 游戏标题和副标题
 * - 新游戏、加载游戏、设置、退出等按钮
 * - 动画效果
 * - 背景音乐控制
 */
class MainMenu : public QWidget
{
    Q_OBJECT

public:
    explicit MainMenu(QWidget* parent = nullptr);
    ~MainMenu();

    // 动画控制
    void showMenu();
    void hideMenu();
    void animateButtons();

signals:
    // 菜单事件信号
    void newGameClicked();
    void loadGameClicked();
    void settingsClicked();
    void exitClicked();

protected:
    // 事件处理
    void paintEvent(QPaintEvent* event) override;
    void showEvent(QShowEvent* event) override;
    void hideEvent(QHideEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;

private slots:
    // 按钮事件处理
    void onNewGameButtonClicked();
    void onLoadGameButtonClicked();
    void onSettingsButtonClicked();
    void onExitButtonClicked();
    
    // 动画处理
    void onButtonAnimationFinished();
    void onBackgroundAnimationUpdate();

private:
    // UI初始化
    void setupUI();
    void createTitle();
    void createButtons();
    void createFooter();
    void setupAnimations();
    void applyStyles();
    
    // 动画效果
    void animateButton(QPushButton* button, int delay);
    void startBackgroundAnimation();
    void stopBackgroundAnimation();

private:
    // UI组件
    QVBoxLayout* m_mainLayout;          ///< 主布局
    QLabel* m_titleLabel;               ///< 标题标签
    QLabel* m_subtitleLabel;            ///< 副标题标签
    QWidget* m_buttonContainer;         ///< 按钮容器
    QVBoxLayout* m_buttonLayout;        ///< 按钮布局
    QLabel* m_copyrightLabel;           ///< 版权信息标签
    
    // 按钮
    QPushButton* m_newGameButton;       ///< 新游戏按钮
    QPushButton* m_loadGameButton;      ///< 加载游戏按钮
    QPushButton* m_settingsButton;      ///< 设置按钮
    QPushButton* m_exitButton;          ///< 退出按钮
    
    // 动画
    QPropertyAnimation* m_titleAnimation;   ///< 标题动画
    QPropertyAnimation* m_subtitleAnimation; ///< 副标题动画
    QTimer* m_buttonAnimationTimer;         ///< 按钮动画定时器
    QTimer* m_backgroundAnimationTimer;     ///< 背景动画定时器
    
    // 动画状态
    int m_currentButtonIndex;           ///< 当前动画按钮索引
    bool m_isAnimating;                 ///< 是否正在动画
    double m_backgroundOffset;          ///< 背景偏移
    
    // 样式
    QString m_buttonStyle;              ///< 按钮样式
    QString m_titleStyle;               ///< 标题样式
    QString m_subtitleStyle;            ///< 副标题样式
};

} // namespace Fantasy

#endif // MAINMENU_H 