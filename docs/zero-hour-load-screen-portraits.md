# Портреты на экране загрузки Zero Hour

«Схватка» и сетевые матчи брали портрет генерала из `ChallengeMode.ini`. У дополнительных фракций Contra Invasion без записи `GeneralPersona` портрет мог отсутствовать. Теперь для каждой фракции можно указать свой портрет в `Data/INI/PlayerTemplate.ini`.

## Настройка

Добавьте необязательное поле `LoadScreenPortrait` в блок нужного `PlayerTemplate`. Значение — имя существующего `MappedImage`. Например, в блоке `FactionChinaSecretWeaponGeneral` можно указать `LoadScreenPortrait = SecGenInv`.

Игра берёт первое доступное изображение в следующем порядке.

1. `LoadScreenPortrait` из `PlayerTemplate.ini`.
2. `BioPortraitLarge` из соответствующего `GeneralPersona` в `ChallengeMode.ini`.
3. Штатная эмблема для базовых USA, China или GLA.
4. `GeneralImage`, затем `SideIconImage` из `PlayerTemplate.ini`.

Поле читает `GeneralsMD/Code/GameEngine/Source/Common/RTS/PlayerTemplate.cpp`. Его значение хранится в `GeneralsMD/Code/GameEngine/Include/Common/PlayerTemplate.h`. `Core/GameEngine/Source/GameClient/GUI/LoadScreen.cpp` использует общий порядок выбора для Skirmish, LAN и GameSpy.

## Проверка

1. Из корня проекта соберите и установите `z_generals` через `.\Tools\Build-Fork.ps1`.
2. Запустите «Схватку» в Contra Invasion по очереди за Armor, Secret Weapon и Fortress. На экране загрузки должны появиться `USA_Superweapon`, `SecGenInv` и `GLA_Assault` соответственно, если для этих фракций не указан `LoadScreenPortrait`.
3. Для проверки приоритета сохраните копию `PlayerTemplate.ini` вне папки мода, временно задайте `LoadScreenPortrait = GLA_Assault` для `FactionChinaSecretWeaponGeneral` и перезапустите игру. После проверки восстановите INI из копии. Для проверки совместимости запустите схватку за генерала с `BioPortraitLarge` и без `LoadScreenPortrait`.

## Откат

Удалите добавленные строки `LoadScreenPortrait` из `PlayerTemplate.ini` или восстановите его из резервной копии. Для отката логики отмените коммит исправления через `git revert` и пересоберите `z_generals`. Если сохранена предыдущая сборка, после закрытия игры можно вернуть её как `generals.exe`.
