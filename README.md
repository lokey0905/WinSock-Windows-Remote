# 安全的Windows遠端系統管理工具設計：使用Winsocket 
- 本項目為網路程式設計課程專題之產物
# 目的
- 目的是想使用WinSocket替代遠端桌面或是SSH的功能，並可以方便的管理機器
# 環境與設定
1. 使用Visual Studio 2022編譯
2. 至屬性管理員新增winsock.props
3. 配置server端port
4. 配置client端連線ip&port
5. 確保server與client在同一網段
6. 確認防火牆設定
# 指令
- /show(Enter) [showChoice] 顯示編號為showChoice的指令輸出結果
- /exit 離開
