#pragma once

#include <QObject>
#include <QString>
#include <QMap>
#include <QVariantMap>
#include <QTimer>
#include <QWidget>
#include <QList>

// 前向声明
class QLabel;
class QProgressBar;
class QPushButton;
class QWidget;

class UIManager : public QObject {
    Q_OBJECT
    
public:
    static UIManager* instance();
    
    // UI状态管理
    void setUIState(const QString& state);
    QString getCurrentState() const { return m_currentState; }
    
    // 角色状态UI
    void updateCharacterStatus(const QString& characterName, const QVariantMap& stats);
    void showCharacterStatus(bool show);
    void setCharacterStatusWidget(QWidget* widget);
    
    // 技能栏UI
    void updateSkillBar(const QList<QVariantMap>& skills);
    void setSkillBarWidget(QWidget* widget);
    void showSkillBar(bool show);
    
    // 背包UI
    void updateInventory(const QList<QVariantMap>& items);
    void setInventoryWidget(QWidget* widget);
    void showInventory(bool show);
    
    // 对话UI
    void showDialogue(const QString& speaker, const QString& content, const QList<QString>& choices);
    void hideDialogue();
    void setDialogueWidget(QWidget* widget);
    
    // 战斗UI
    void showBattleUI(bool show);
    void updateBattleStatus(const QVariantMap& playerStats, const QList<QVariantMap>& enemyStats);
    void setBattleWidget(QWidget* widget);
    
    // 小地图UI
    void updateMinimap(const QVariantMap& mapData);
    void setMinimapWidget(QWidget* widget);
    void showMinimap(bool show);
    
    // 任务UI
    void updateQuestLog(const QList<QVariantMap>& quests);
    void setQuestWidget(QWidget* widget);
    void showQuestLog(bool show);
    
    // 设置UI
    void showSettings(bool show);
    void setSettingsWidget(QWidget* widget);
    
    // 通知系统
    void showNotification(const QString& message, const QString& type = "info", int duration = 3000);
    void showTooltip(const QString& text, const QPoint& position);
    void hideTooltip();
    
    // 加载界面
    void showLoadingScreen(const QString& message = "加载中...");
    void hideLoadingScreen();
    void updateLoadingProgress(int progress);
    
    // 暂停菜单
    void showPauseMenu(bool show);
    void setPauseMenuWidget(QWidget* widget);
    
    // 游戏结束界面
    void showGameOver(bool victory, const QString& message = "");
    void setGameOverWidget(QWidget* widget);
    
    // UI动画
    void fadeInWidget(QWidget* widget, int duration = 300);
    void fadeOutWidget(QWidget* widget, int duration = 300);
    void slideInWidget(QWidget* widget, const QString& direction = "right", int duration = 300);
    void slideOutWidget(QWidget* widget, const QString& direction = "right", int duration = 300);
    
    // 主题和样式
    void setTheme(const QString& theme);
    QString getCurrentTheme() const { return m_currentTheme; }
    void applyStyleSheet(QWidget* widget, const QString& style);
    
    // 本地化
    void setLanguage(const QString& language);
    QString getCurrentLanguage() const { return m_currentLanguage; }
    QString translate(const QString& key);
    
signals:
    void uiStateChanged(const QString& state);
    void characterStatusUpdated(const QString& characterName, const QVariantMap& stats);
    void skillBarUpdated(const QList<QVariantMap>& skills);
    void inventoryUpdated(const QList<QVariantMap>& items);
    void dialogueShown(const QString& speaker, const QString& content);
    void dialogueHidden();
    void choiceSelected(int choiceIndex);
    void battleUIUpdated(const QVariantMap& playerStats, const QList<QVariantMap>& enemyStats);
    void minimapUpdated(const QVariantMap& mapData);
    void questLogUpdated(const QList<QVariantMap>& quests);
    void notificationShown(const QString& message, const QString& type);
    void loadingProgressUpdated(int progress);
    void themeChanged(const QString& theme);
    void languageChanged(const QString& language);
    
private slots:
    void updateUI(float deltaTime);
    void onNotificationTimeout();
    void onTooltipTimeout();
    
private:
    UIManager();
    ~UIManager();
    
    void initializeUI();
    void setupConnections();
    void createDefaultWidgets();
    void applyDefaultStyles();
    
    // UI组件
    QWidget* m_characterStatusWidget;
    QWidget* m_skillBarWidget;
    QWidget* m_inventoryWidget;
    QWidget* m_dialogueWidget;
    QWidget* m_battleWidget;
    QWidget* m_minimapWidget;
    QWidget* m_questWidget;
    QWidget* m_settingsWidget;
    QWidget* m_pauseMenuWidget;
    QWidget* m_gameOverWidget;
    QWidget* m_loadingWidget;
    QWidget* m_notificationWidget;
    QWidget* m_tooltipWidget;
    
    // 状态管理
    QString m_currentState;
    QString m_currentTheme;
    QString m_currentLanguage;
    
    // 数据缓存
    QVariantMap m_characterStats;
    QList<QVariantMap> m_skillList;
    QList<QVariantMap> m_inventoryItems;
    QList<QVariantMap> m_questList;
    QVariantMap m_battleData;
    QVariantMap m_minimapData;
    
    // 定时器
    QTimer* m_updateTimer;
    QTimer* m_notificationTimer;
    QTimer* m_tooltipTimer;
    
    // 本地化数据
    QMap<QString, QString> m_translations;
    
    // 动画状态
    QMap<QWidget*, QTimer*> m_animationTimers;
    
    static UIManager* s_instance;
};
