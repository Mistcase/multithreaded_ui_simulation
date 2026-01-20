# Пример работы generation

## Начальное состояние

```
Создана нода textId = MakeNodeId(1, 0)  // index=1, generation=0

renderTexts_[1] = {x: 10, y: 20, text: "Hello"}
textGenerations_[1] = 0
```

## Кадр 1: Изменение позиции

**Update поток:**
```cpp
text_->SetPosition(15, 25);  // Пишет в ChangeBuffer
```

**Sync():**
```cpp
// ChangeBuffer.version = 1
// textGenerations_[1] = 0 (не меняется!)
// renderTexts_[1] = {x: 15, y: 25, text: "Hello"}  // Обновлено
```

**Render поток:**
```cpp
TryGetText(textId) → проверяет: textGenerations_[1] == 0? ДА → возвращает &renderTexts_[1]
// Успешно читает обновлённые данные
```

## Кадр 2: Удаление ноды

**Update поток:**
```cpp
text_->Term();  // Помечает deleted = true в ChangeBuffer
```

**Sync():**
```cpp
// ChangeBuffer.version = 2
// textGenerations_[1] = 0 → 1  (ИНКРЕМЕНТИРОВАН!)
// renderTexts_[1] = {}  // Очищено
```

**Render поток (в это же время или позже):**
```cpp
// У кого-то остался старый handle textId = (index=1, gen=0)
TryGetText(textId) → проверяет: textGenerations_[1] == 0? НЕТ (там теперь 1) → возвращает nullptr
// Защита от доступа к удалённой ноде!
```

## Кадр 3: Создание новой ноды в том же слоте

**Update поток:**
```cpp
// Создана новая нода с textId2 = MakeNodeId(1, 1)  // index=1, generation=1
text2_->SetPosition(100, 200);
```

**Sync():**
```cpp
// ChangeBuffer.version = 3
// textGenerations_[1] = 1 (уже было)
// renderTexts_[1] = {x: 100, y: 200, text: "New"}  // Новая нода
```

**Render поток:**
```cpp
// Старый handle textId = (index=1, gen=0) всё ещё невалиден
TryGetText(textId) → textGenerations_[1] == 0? НЕТ → nullptr

// Новый handle textId2 = (index=1, gen=1) валиден
TryGetText(textId2) → textGenerations_[1] == 1? ДА → &renderTexts_[1]
```

## Проблема БЕЗ generation

**Без generation:**
```cpp
// После удаления textId
renderTexts_[1] = {}  // Очищено

// Но старый handle textId = (index=1) всё ещё указывает на слот 1
TryGetText(textId) → &renderTexts_[1]  // Возвращает пустую/старую ноду!

// Если создали новую ноду в слоте 1:
renderTexts_[1] = {x: 100, y: 200}  // Новая нода

// Старый handle теперь указывает на НОВУЮ ноду (баг!)
TryGetText(textId) → &renderTexts_[1]  // Читает данные новой ноды как свои!
```

**С generation:**
```cpp
// После удаления
textGenerations_[1] = 1  // Инкрементировано

// Старый handle textId = (index=1, gen=0)
TryGetText(textId) → textGenerations_[1] == 0? НЕТ → nullptr  // Защита!

// Новая нода textId2 = (index=1, gen=1)
TryGetText(textId2) → textGenerations_[1] == 1? ДА → &renderTexts_[1]  // Работает
```

## Пример с детьми

**Начало:**
```cpp
rootId = MakeNodeId(0, 0)  // index=0, gen=0
textId = MakeNodeId(1, 0)  // index=1, gen=0

root->AddChild(textId, true);
// renderContainers_[0].children = [{id: textId, text: &renderTexts_[1]}]
```

**Удаление textId:**
```cpp
text_->Term();
// Sync():
// textGenerations_[1] = 0 → 1
// renderTexts_[1] = {}
```

**Render поток обходит дерево:**
```cpp
for (auto& ch : root->children) {
    if (ch.isText) {
        // ch.id = textId = (index=1, gen=0) - старый handle!
        const RenderTextNode* t = TryGetText(ch.id);
        // Проверка: textGenerations_[1] == 0? НЕТ (там 1) → nullptr
        if (t) {  // Не выполнится - защита от доступа к удалённой ноде
            // ...
        }
    }
}
```

**Без generation:**
```cpp
// ch.text всё ещё указывает на &renderTexts_[1]
// Но renderTexts_[1] уже очищено или переиспользовано!
// Доступ к мусору или данным другой ноды → краш/баг
```
