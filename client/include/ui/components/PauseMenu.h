/**
 * @file PauseMenu.h
 * @brief 暂停菜单组件
 * @author [pengchengkang]
 * @date 2025.06.17
 */

#ifndef PAUSEMENU_H
#define PAUSEMENU_H

#include <QWidget>
#include <QVBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QGraphicsDropShadowEffect>
#include <QPropertyAnimation>

namespace Fantasy {

/**
 * @brief 暂停菜单组件
 * 
 * 提供游戏暂停时的菜单界面，包括：
 * - 暂停标题
 * - 继续游戏、保存游戏、设置、返回主菜单等按钮
 * - 半透明背景效果
 * - 动画效果
 */
class PauseMenu : public QWidget
{
    Q_OBJECT

public:
    explicit PauseMenu(QWidget* parent = nullptr);
    ~PauseMenu();

    // 动画控制
    void showMenu();
    void hideMenu();

signals:
    // 菜单事件信号
    void resumeClicked();
    void saveGameClicked();
    void loadGameClicked();
    void settingsClicked();
    void backToMainMenuClicked();

protected:
    // 事件处理
    void paintEvent(QPaintEvent* event) override;
    void showEvent(QShowEvent* event) override;
    void hideEvent(QHideEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;

private slots:
    // 按钮事件处理
    void onResumeButtonClicked();
    void onSaveGameButtonClicked();
    void onLoadGameButtonClicked();
    void onSettingsButtonClicked();
    void onBackToMainMenuButtonClicked();

private:
    // UI初始化
    void setupUI();
    void createTitle();
    void createButtons();
    void setupAnimations();
    void applyStyles();

private:
    // UI组件
    QVBoxLayout* m_mainLayout;          ///< 主布局
    QLabel* m_titleLabel;               ///< 标题标签
    QWidget* m_buttonContainer;         ///< 按钮容器
    QVBoxLayout* m_buttonLayout;        ///< 按钮布局
    
    // 按钮
    QPushButton* m_resumeButton;        ///< 继续游戏按钮
    QPushButton* m_saveGameButton;      ///< 保存游戏按钮
    QPushButton* m_loadGameButton;      ///< 加载游戏按钮
    QPushButton* m_settingsButton;      ///< 设置按钮
    QPushButton* m_backToMainMenuButton; ///< 返回主菜单按钮
    
    // 动画
    QPropertyAnimation* m_fadeInAnimation;  ///< 淡入动画
    QPropertyAnimation* m_fadeOutAnimation; ///< 淡出动画
    
    // 样式
    QString m_buttonStyle;              ///< 按钮样式
    QString m_titleStyle;               ///< 标题样式
};

} // namespace Fantasy

#endif // PAUSEMENU_H 