#include "MainWindow.h"
#include <QApplication>
#include <QMenuBar>
#include <QStatusBar>
#include <QMessageBox>
#include <QCloseEvent>
#include <QUuid>
#include <QDebug>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , m_centralWidget(nullptr)
    , m_currentStickerId("")
{
    setWindowTitle("桌面贴纸管理器");
    setMinimumSize(800, 600);
    resize(1000, 700);

    setupUI();
    setupMenuBar();
    setupStatusBar();

    // 连接信号
    connect(m_createStickerBtn, &QPushButton::clicked, this, &MainWindow::onCreateStickerClicked);
    connect(m_deleteStickerBtn, &QPushButton::clicked, this, &MainWindow::onDeleteStickerClicked);
    connect(m_editStickerBtn, &QPushButton::clicked, this, &MainWindow::onEditStickerClicked);
    connect(m_stickerList, &QListWidget::itemSelectionChanged, this, &MainWindow::onStickerListSelectionChanged);

    connect(m_browseImageBtn, &QPushButton::clicked, this, &MainWindow::onBrowseImageClicked);
    connect(m_addEventBtn, &QPushButton::clicked, this, &MainWindow::onAddEventClicked);
    connect(m_removeEventBtn, &QPushButton::clicked, this, &MainWindow::onRemoveEventClicked);
    connect(m_eventTable, &QTableWidget::itemChanged, this, &MainWindow::onEventTableItemChanged);

    connect(m_applyChangesBtn, &QPushButton::clicked, this, &MainWindow::onApplyChangesClicked);
    connect(m_loadConfigBtn, &QPushButton::clicked, this, &MainWindow::onLoadConfigClicked);
    connect(m_saveConfigBtn, &QPushButton::clicked, this, &MainWindow::onSaveConfigClicked);

    // 初始化
    clearStickerEditor();

    qDebug() << "主窗口创建完成";
}

MainWindow::~MainWindow()
{
    qDebug() << "主窗口销毁";
}

void MainWindow::setupUI()
{
    m_centralWidget = new QWidget;
    setCentralWidget(m_centralWidget);

    m_mainSplitter = new QSplitter(Qt::Horizontal);

    setupStickerList();
    setupStickerEditor();

    m_mainSplitter->addWidget(m_stickerListWidget);
    m_mainSplitter->addWidget(m_editorWidget);
    m_mainSplitter->setStretchFactor(0, 1);
    m_mainSplitter->setStretchFactor(1, 2);

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addWidget(m_mainSplitter);

    // 控制按钮
    QHBoxLayout *buttonLayout = new QHBoxLayout;
    m_loadConfigBtn = new QPushButton("加载配置");
    m_saveConfigBtn = new QPushButton("保存配置");
    m_applyChangesBtn = new QPushButton("应用更改");

    buttonLayout->addWidget(m_loadConfigBtn);
    buttonLayout->addWidget(m_saveConfigBtn);
    buttonLayout->addStretch();
    buttonLayout->addWidget(m_applyChangesBtn);

    mainLayout->addLayout(buttonLayout);

    m_centralWidget->setLayout(mainLayout);
}

void MainWindow::setupStickerList()
{
    m_stickerListWidget = new QWidget;
    QVBoxLayout *layout = new QVBoxLayout(m_stickerListWidget);

    // 标题
    QLabel *titleLabel = new QLabel("贴纸列表");
    titleLabel->setStyleSheet("font-weight: bold; font-size: 14px; padding: 5px;");
    layout->addWidget(titleLabel);

    // 贴纸列表
    m_stickerList = new QListWidget;
    layout->addWidget(m_stickerList);

    // 按钮
    QHBoxLayout *buttonLayout = new QHBoxLayout;
    m_createStickerBtn = new QPushButton("创建贴纸");
    m_editStickerBtn = new QPushButton("编辑贴纸");
    m_deleteStickerBtn = new QPushButton("删除贴纸");

    m_editStickerBtn->setEnabled(false);
    m_deleteStickerBtn->setEnabled(false);

    buttonLayout->addWidget(m_createStickerBtn);
    buttonLayout->addWidget(m_editStickerBtn);
    buttonLayout->addWidget(m_deleteStickerBtn);

    layout->addLayout(buttonLayout);
}

void MainWindow::setupStickerEditor()
{
    m_editorWidget = new QWidget;
    QVBoxLayout *layout = new QVBoxLayout(m_editorWidget);

    // 标题
    QLabel *titleLabel = new QLabel("贴纸编辑器");
    titleLabel->setStyleSheet("font-weight: bold; font-size: 14px; padding: 5px;");
    layout->addWidget(titleLabel);

    // 标签页
    m_editorTabs = new QTabWidget;

    // 基本属性标签页
    m_basicTab = new QWidget;
    setupBasicTab();
    m_editorTabs->addTab(m_basicTab, "基本属性");

    // 事件编辑标签页
    m_eventsTab = new QWidget;
    setupEventEditor();
    m_editorTabs->addTab(m_eventsTab, "事件配置");

    layout->addWidget(m_editorTabs);
}

void MainWindow::setupBasicTab()
{
    QVBoxLayout *layout = new QVBoxLayout(m_basicTab);

    // 基本信息组
    QGroupBox *basicGroup = new QGroupBox("基本信息");
    QGridLayout *basicLayout = new QGridLayout(basicGroup);

    basicLayout->addWidget(new QLabel("名称:"), 0, 0);
    m_nameEdit = new QLineEdit;
    basicLayout->addWidget(m_nameEdit, 0, 1, 1, 2);

    basicLayout->addWidget(new QLabel("图片路径:"), 1, 0);
    m_imagePathEdit = new QLineEdit;
    m_browseImageBtn = new QPushButton("浏览...");
    basicLayout->addWidget(m_imagePathEdit, 1, 1);
    basicLayout->addWidget(m_browseImageBtn, 1, 2);

    layout->addWidget(basicGroup);

    // 位置和大小组
    QGroupBox *positionGroup = new QGroupBox("位置和大小");
    QGridLayout *positionLayout = new QGridLayout(positionGroup);

    positionLayout->addWidget(new QLabel("X坐标:"), 0, 0);
    m_xSpinBox = new QSpinBox;
    m_xSpinBox->setRange(-9999, 9999);
    positionLayout->addWidget(m_xSpinBox, 0, 1);

    positionLayout->addWidget(new QLabel("Y坐标:"), 0, 2);
    m_ySpinBox = new QSpinBox;
    m_ySpinBox->setRange(-9999, 9999);
    positionLayout->addWidget(m_ySpinBox, 0, 3);

    positionLayout->addWidget(new QLabel("宽度:"), 1, 0);
    m_widthSpinBox = new QSpinBox;
    m_widthSpinBox->setRange(50, 1000);
    positionLayout->addWidget(m_widthSpinBox, 1, 1);

    positionLayout->addWidget(new QLabel("高度:"), 1, 2);
    m_heightSpinBox = new QSpinBox;
    m_heightSpinBox->setRange(50, 1000);
    positionLayout->addWidget(m_heightSpinBox, 1, 3);

    layout->addWidget(positionGroup);

    // 显示属性组
    QGroupBox *displayGroup = new QGroupBox("显示属性");
    QGridLayout *displayLayout = new QGridLayout(displayGroup);

    displayLayout->addWidget(new QLabel("透明度:"), 0, 0);
    m_opacitySpinBox = new QDoubleSpinBox;
    m_opacitySpinBox->setRange(0.1, 1.0);
    m_opacitySpinBox->setSingleStep(0.1);
    m_opacitySpinBox->setValue(1.0);
    displayLayout->addWidget(m_opacitySpinBox, 0, 1);

    m_visibleCheckBox = new QCheckBox("可见");
    m_visibleCheckBox->setChecked(true);
    displayLayout->addWidget(m_visibleCheckBox, 0, 2);

    m_desktopModeCheckBox = new QCheckBox("桌面模式");
    m_desktopModeCheckBox->setChecked(true);
    displayLayout->addWidget(m_desktopModeCheckBox, 1, 0);

    // 新增控制选项
    m_allowDragCheckBox = new QCheckBox("允许拖动");
    m_allowDragCheckBox->setChecked(true);
    displayLayout->addWidget(m_allowDragCheckBox, 1, 1);

    m_clickThroughCheckBox = new QCheckBox("点击穿透");
    m_clickThroughCheckBox->setChecked(false);
    displayLayout->addWidget(m_clickThroughCheckBox, 1, 2);

    layout->addWidget(displayGroup);

    layout->addStretch();
}

void MainWindow::setupEventEditor()
{
    QVBoxLayout *layout = new QVBoxLayout(m_eventsTab);

    // 事件表格
    m_eventTable = new QTableWidget;
    m_eventTable->setColumnCount(5);
    QStringList headers = {"触发器", "事件类型", "目标", "参数", "启用"};
    m_eventTable->setHorizontalHeaderLabels(headers);
    m_eventTable->horizontalHeader()->setStretchLastSection(true);
    layout->addWidget(m_eventTable);

    // 添加事件控制
    QGroupBox *addEventGroup = new QGroupBox("添加新事件");
    QGridLayout *addLayout = new QGridLayout(addEventGroup);

    addLayout->addWidget(new QLabel("触发器:"), 0, 0);
    m_triggerComboBox = new QComboBox;
    m_triggerComboBox->addItems({"左键单击", "右键单击", "双击", "滚轮向上", "滚轮向下", "鼠标进入", "鼠标离开"});
    addLayout->addWidget(m_triggerComboBox, 0, 1);

    addLayout->addWidget(new QLabel("事件类型:"), 0, 2);
    m_eventTypeComboBox = new QComboBox;
    m_eventTypeComboBox->addItems({"打开程序", "打开文件夹", "打开文件", "播放声音", "显示消息", "自定义脚本"});
    addLayout->addWidget(m_eventTypeComboBox, 0, 3);

    addLayout->addWidget(new QLabel("目标:"), 1, 0);
    m_targetEdit = new QLineEdit;
    addLayout->addWidget(m_targetEdit, 1, 1, 1, 2);

    addLayout->addWidget(new QLabel("参数:"), 1, 3);
    m_parametersEdit = new QLineEdit;
    addLayout->addWidget(m_parametersEdit, 1, 4);

    layout->addWidget(addEventGroup);

    // 按钮
    QHBoxLayout *buttonLayout = new QHBoxLayout;
    m_addEventBtn = new QPushButton("添加事件");
    m_removeEventBtn = new QPushButton("删除事件");

    buttonLayout->addWidget(m_addEventBtn);
    buttonLayout->addWidget(m_removeEventBtn);
    buttonLayout->addStretch();

    layout->addLayout(buttonLayout);
}

void MainWindow::setupMenuBar()
{
    QMenu *fileMenu = menuBar()->addMenu("文件");
    fileMenu->addAction("加载配置", this, &MainWindow::onLoadConfigClicked);
    fileMenu->addAction("保存配置", this, &MainWindow::onSaveConfigClicked);
    fileMenu->addSeparator();
    fileMenu->addAction("退出", this, &MainWindow::onExitClicked);

    QMenu *editMenu = menuBar()->addMenu("编辑");
    editMenu->addAction("创建贴纸", this, &MainWindow::onCreateStickerClicked);
    editMenu->addAction("删除贴纸", this, &MainWindow::onDeleteStickerClicked);

    QMenu *helpMenu = menuBar()->addMenu("帮助");
    helpMenu->addAction("关于", [this]() {
        QMessageBox::about(this, "关于", "桌面贴纸管理器 v1.0\n\n一个简单易用的桌面贴纸工具");
    });
}

void MainWindow::onExitClicked()
{
    QMessageBox::StandardButton reply = QMessageBox::question(
        this, "退出程序",
        "确定要退出桌面贴纸管理器吗？\n所有贴纸将被关闭。",
        QMessageBox::Yes | QMessageBox::No
    );

    if (reply == QMessageBox::Yes) {
        emit exitRequested();
    }
}

void MainWindow::setupStatusBar()
{
    statusBar()->showMessage("就绪");
}

void MainWindow::showAndRaise()
{
    show();
    raise();
    activateWindow();
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    hide();
    event->ignore(); // 阻止关闭，只隐藏窗口
}

void MainWindow::forceClose()
{
    // 强制关闭窗口（用于程序退出时）
    qDebug() << "主窗口强制关闭";
    QWidget::close();
}

void MainWindow::onCreateStickerClicked()
{
    StickerConfig config;
    config.id = QUuid::createUuid().toString(QUuid::WithoutBraces);
    config.name = QString("贴纸 %1").arg(m_stickerConfigs.size() + 1);
    config.position = QPoint(100, 100);
    config.size = QSize(200, 200);
    config.isDesktopMode = true;
    config.visible = true;
    config.opacity = 1.0;

    emit createSticker(config);
}

void MainWindow::onDeleteStickerClicked()
{
    if (m_currentStickerId.isEmpty()) {
        QMessageBox::warning(this, "警告", "请先选择要删除的贴纸");
        return;
    }

    QMessageBox::StandardButton reply = QMessageBox::question(
        this, "删除贴纸",
        "确定要删除选中的贴纸吗？",
        QMessageBox::Yes | QMessageBox::No
    );

    if (reply == QMessageBox::Yes) {
        emit deleteSticker(m_currentStickerId);
    }
}

void MainWindow::onEditStickerClicked()
{
    if (m_currentStickerId.isEmpty()) {
        QMessageBox::warning(this, "警告", "请先选择要编辑的贴纸");
        return;
    }

    // 编辑功能已经通过选择列表项实现，这里可以切换到编辑标签页
    m_editorTabs->setCurrentWidget(m_basicTab);
}

void MainWindow::onStickerListSelectionChanged()
{
    QList<QListWidgetItem*> selected = m_stickerList->selectedItems();

    if (selected.isEmpty()) {
        m_currentStickerId = "";
        m_editStickerBtn->setEnabled(false);
        m_deleteStickerBtn->setEnabled(false);
        clearStickerEditor();
    } else {
        QString stickerId = selected.first()->data(Qt::UserRole).toString();
        m_currentStickerId = stickerId;
        m_editStickerBtn->setEnabled(true);
        m_deleteStickerBtn->setEnabled(true);

        // 更新编辑器
        for (const auto &config : m_stickerConfigs) {
            if (config.id == stickerId) {
                updateStickerEditor(config);
                break;
            }
        }
    }
}

void MainWindow::onBrowseImageClicked()
{
    QString fileName = QFileDialog::getOpenFileName(
        this,
        "选择贴纸图片",
        QStandardPaths::writableLocation(QStandardPaths::PicturesLocation),
        "图像文件 (*.png *.jpg *.jpeg *.bmp *.gif *.svg)"
    );

    if (!fileName.isEmpty()) {
        m_imagePathEdit->setText(fileName);
    }
}

void MainWindow::onAddEventClicked()
{
    if (m_currentStickerId.isEmpty()) {
        QMessageBox::warning(this, "警告", "请先选择一个贴纸");
        return;
    }

    if (m_targetEdit->text().isEmpty()) {
        QMessageBox::warning(this, "警告", "请输入事件目标");
        return;
    }

    StickerEvent event;
    event.trigger = stringToMouseTrigger(m_triggerComboBox->currentText());
    event.type = stringToEventType(m_eventTypeComboBox->currentText());
    event.target = m_targetEdit->text();
    event.parameters = m_parametersEdit->text();
    event.enabled = true;

    // 添加到当前贴纸配置
    for (auto &config : m_stickerConfigs) {
        if (config.id == m_currentStickerId) {
            config.events.append(event);
            updateEventTable();
            break;
        }
    }

    // 清空输入框
    m_targetEdit->clear();
    m_parametersEdit->clear();

    statusBar()->showMessage("事件已添加", 2000);
}

void MainWindow::onRemoveEventClicked()
{
    int currentRow = m_eventTable->currentRow();
    if (currentRow >= 0 && !m_currentStickerId.isEmpty()) {
        for (auto &config : m_stickerConfigs) {
            if (config.id == m_currentStickerId) {
                if (currentRow < config.events.size()) {
                    config.events.removeAt(currentRow);
                    updateEventTable();
                    statusBar()->showMessage("事件已删除", 2000);
                }
                break;
            }
        }
    } else {
        QMessageBox::warning(this, "警告", "请选择要删除的事件");
    }
}

void MainWindow::onEventTableItemChanged(QTableWidgetItem *item)
{
    if (!item || m_currentStickerId.isEmpty()) {
        return;
    }

    int row = item->row();
    int column = item->column();

    // 找到对应的配置
    for (auto &config : m_stickerConfigs) {
        if (config.id == m_currentStickerId) {
            if (row < config.events.size()) {
                StickerEvent &event = config.events[row];

                // 根据列更新事件数据
                switch (column) {
                case 0: // 触发器
                    event.trigger = stringToMouseTrigger(item->text());
                    break;
                case 1: // 事件类型
                    event.type = stringToEventType(item->text());
                    break;
                case 2: // 目标
                    event.target = item->text();
                    break;
                case 3: // 参数
                    event.parameters = item->text();
                    break;
                case 4: // 启用状态
                    event.enabled = (item->checkState() == Qt::Checked);
                    break;
                }

                statusBar()->showMessage("事件已更新", 2000);
            }
            break;
        }
    }
}

void MainWindow::onApplyChangesClicked()
{
    if (m_currentStickerId.isEmpty()) {
        QMessageBox::warning(this, "警告", "请先选择一个贴纸");
        return;
    }

    StickerConfig config = getStickerConfigFromEditor();

    // 更新本地配置
    for (auto &storedConfig : m_stickerConfigs) {
        if (storedConfig.id == m_currentStickerId) {
            storedConfig = config;
            break;
        }
    }

    // 发出编辑信号，传递完整的配置
    emit editStickerWithConfig(m_currentStickerId, config);
    statusBar()->showMessage("配置已应用", 3000);
}

void MainWindow::onLoadConfigClicked()
{
    emit loadStickerConfig();
}

void MainWindow::onSaveConfigClicked()
{
    emit saveStickerConfig();
}

void MainWindow::onStickerCreated(const StickerConfig &config)
{
    // 将新配置添加到本地列表
    m_stickerConfigs.append(config);
    updateStickerList();
    statusBar()->showMessage(QString("贴纸 '%1' 已创建").arg(config.name), 3000);
}

void MainWindow::onStickerDeleted(const QString &stickerId)
{
    // 从本地配置中移除
    for (int i = m_stickerConfigs.size() - 1; i >= 0; --i) {
        if (m_stickerConfigs[i].id == stickerId) {
            m_stickerConfigs.removeAt(i);
            break;
        }
    }

    if (m_currentStickerId == stickerId) {
        m_currentStickerId = "";
        clearStickerEditor();
    }

    updateStickerList();
    statusBar()->showMessage("贴纸已删除", 3000);
}

void MainWindow::onStickerConfigChanged(const StickerConfig &config)
{
    // 更新本地配置
    for (auto &storedConfig : m_stickerConfigs) {
        if (storedConfig.id == config.id) {
            storedConfig = config;
            break;
        }
    }

    updateStickerList();

    // 如果是当前选中的贴纸，更新编辑器
    if (m_currentStickerId == config.id) {
        updateStickerEditor(config);
    }
}

void MainWindow::updateStickerEditor(const StickerConfig &config)
{
    m_nameEdit->setText(config.name);
    m_imagePathEdit->setText(config.imagePath);
    m_xSpinBox->setValue(config.position.x());
    m_ySpinBox->setValue(config.position.y());
    m_widthSpinBox->setValue(config.size.width());
    m_heightSpinBox->setValue(config.size.height());
    m_opacitySpinBox->setValue(config.opacity);
    m_visibleCheckBox->setChecked(config.visible);
    m_desktopModeCheckBox->setChecked(config.isDesktopMode);
    m_allowDragCheckBox->setChecked(config.allowDrag);        // 新增
    m_clickThroughCheckBox->setChecked(config.clickThrough);  // 新增

    updateEventTable();
}

void MainWindow::clearStickerEditor()
{
    m_nameEdit->clear();
    m_imagePathEdit->clear();
    m_xSpinBox->setValue(0);
    m_ySpinBox->setValue(0);
    m_widthSpinBox->setValue(200);
    m_heightSpinBox->setValue(200);
    m_opacitySpinBox->setValue(1.0);
    m_visibleCheckBox->setChecked(true);
    m_desktopModeCheckBox->setChecked(true);
    m_allowDragCheckBox->setChecked(true);      // 新增
    m_clickThroughCheckBox->setChecked(false);  // 新增

    m_eventTable->setRowCount(0);
}

StickerConfig MainWindow::getStickerConfigFromEditor() const
{
    StickerConfig config;

    // 获取现有配置作为基础
    for (const auto &storedConfig : m_stickerConfigs) {
        if (storedConfig.id == m_currentStickerId) {
            config = storedConfig; // 保留ID和事件
            break;
        }
    }

    // 更新编辑器中的值
    config.name = m_nameEdit->text();
    config.imagePath = m_imagePathEdit->text();
    config.position = QPoint(m_xSpinBox->value(), m_ySpinBox->value());
    config.size = QSize(m_widthSpinBox->value(), m_heightSpinBox->value());
    config.opacity = m_opacitySpinBox->value();
    config.visible = m_visibleCheckBox->isChecked();
    config.isDesktopMode = m_desktopModeCheckBox->isChecked();
    config.allowDrag = m_allowDragCheckBox->isChecked();        // 新增
    config.clickThrough = m_clickThroughCheckBox->isChecked();  // 新增

    return config;
}

void MainWindow::updateEventTable()
{
    if (m_currentStickerId.isEmpty()) {
        m_eventTable->setRowCount(0);
        return;
    }

    const StickerConfig *config = nullptr;
    for (const auto &storedConfig : m_stickerConfigs) {
        if (storedConfig.id == m_currentStickerId) {
            config = &storedConfig;
            break;
        }
    }

    if (!config) {
        return;
    }

    m_eventTable->setRowCount(config->events.size());

    for (int i = 0; i < config->events.size(); ++i) {
        const auto &event = config->events[i];

        m_eventTable->setItem(i, 0, new QTableWidgetItem(mouseTriggersToString(event.trigger)));
        m_eventTable->setItem(i, 1, new QTableWidgetItem(eventTypeToString(event.type)));
        m_eventTable->setItem(i, 2, new QTableWidgetItem(event.target));
        m_eventTable->setItem(i, 3, new QTableWidgetItem(event.parameters));

        QTableWidgetItem *enabledItem = new QTableWidgetItem();
        enabledItem->setCheckState(event.enabled ? Qt::Checked : Qt::Unchecked);
        m_eventTable->setItem(i, 4, enabledItem);
    }
}

void MainWindow::updateStickerList()
{
    QString currentId = m_currentStickerId; // 保存当前选择
    m_stickerList->clear();

    for (const StickerConfig &config : m_stickerConfigs) {
        QListWidgetItem *item = new QListWidgetItem(config.name);
        item->setData(Qt::UserRole, config.id);

        if (!config.visible) {
            item->setTextColor(QColor(128, 128, 128));
        }

        m_stickerList->addItem(item);

        // 恢复选择
        if (config.id == currentId) {
            item->setSelected(true);
        }
    }

    // 更新状态栏
    statusBar()->showMessage(QString("共 %1 个贴纸").arg(m_stickerConfigs.size()), 3000);
}

void MainWindow::onStickerConfigsUpdated(const QList<StickerConfig> &configs)
{
    qDebug() << "更新贴纸列表，共" << configs.size() << "个贴纸";

    m_stickerConfigs = configs;
    updateStickerList();
}
