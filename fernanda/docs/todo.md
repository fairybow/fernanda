# To-Do

## General
- [ ] Redo Dependency Tree
- [ ] Moveable documents folder
- [ ] Popup if temps exist on start (but would need to reload last project first, and then check if temps exist?)
- [ ] Add dialogs to free functions?
- [ ] Move the startup `ColorBar` singleshot to a different window event? `showEvent` doesn't seem to be working for this
- [ ] Move the `nullptr` checks closer to the functions that create the `nullptr`, if possible (viz., don't wait till the info is sent to `Story` to cancel a nullptr from `Pane renameItem()`)
- [ ] Bombine all the messagebox functions into something that generates most of it as a default config
- [x] ~~Change/remove all "`metaDoc`" names~~
- [ ] Replace certain bool args with enums for descriptive actions taken (like "finalize" in `dom->renames()`)
- [ ] Alphabetize enums and generally clean up headers
- [ ] Custom highlight colors for line highlight and `Delegate` highlight (like with cursor)?
- [ ] Remove the regular-scrolls-toggled-off for no theme; can just style appropriately in base
- [ ] Add border variables to stylesheets--for the most part, these can just be `transparent`, but allows for user to decide in customs
- [ ] Hold current story file from being edited while Fernanda is open? (Not sure this is possible. Plus, will need a way to temporarily request access for `Archiver` for editing)
- [ ] Export selected / all
- [ ] Total word count (export-marked or all)
- [x] ~~Make Pallete.txt an .md~~
- [ ] Is there a way to link swatches?
- [ ] Implement QStringLiterals where possible

## Classes

### Archiver
- [ ] Switch to streams (i.e., for `add()` and `create()`)
- [ ] Rename/refactor functions to better reflect their roles / be more descriptive

### ColorBar

### Dom
- [ ] Mark files as exportable

### Fernanda (MainWindow)

- [x] ~~View menu function is gross and bad~~
- [x] ~~Combine two cursor toggles into one submenu~~
- [x] ~~Toggle-specific menu~~
- [ ] Separate menu into its own class?
- [ ] Auto-hide menu option
- [x] ~~Order the items in menus Window and Editor (and General, if applicable)~~
- [ ] Rename menu locals to reflect the alphabetical order of the items
- [ ] Rename `viewToggles()` to match
- [x] ~~Add path to user data folder to the sample themes popup~~
- [x] ~~^ Or a button to open UD folder?~~
- [x] ~~Open UD folders from Help menu~~
- [x] ~~Make Dev menu instead of Status Bar items~~
- [x] ~~I don't think `toggleWidget()` or `toggleGlobals()` are slots~~

### Editor / LineNumberArea
- [x] ~~Remove annoying white block under cursor -_-~~
- [ ] It is not clear to me that `updateLineNumberAreaWidth(int newBlockCount)` actually uses `newBlockCount` arg
- [ ] Save undo/redo stacks
- [x] ~~Toggle chonky cursor vs regular~~
- [x] ~~Toggle cursor blink~~
- [ ] Editor spacing and kerning sliders
- [ ] Arrow keys follow block strangely
- [ ] `LineNumberArea` is not showing up initially on blank documents
- [ ] ^ I can't tell if the above is fixed. I have zero idea what would have fixed it, but it seems fixed.
- [x] ~~Avoid passing entire document for cursor underpaint lol~~
- [ ] Wrap for parentheses and other closables
- [ ] If a filter was just applied, backspace should function as undo
- [ ] Style horizontal scrollbar
- [ ] Make thin cursor change color when there's a selection?
- [x] ~~Block cursor should default to char width, then to average char width~~

### Indicator
- [ ] Deactivate for extremely large strings / convert to non-automatic counting (refresh symbol)

### Keyfilter
- [ ] Auto-format ellipses
- [ ] Auto-delete spaces before punctuation
- [ ] Quote wrapping does not account for non-American punctuation/quote order
- [ ] Add guillemets (both versions)?
- [ ] On enter-press if the next `char` is closing parenthesis, quote, etc., then apply a skip before new line?
- [x] ~~Avoid passing entire document lol~~

### Pane
- [ ] Persist selected-item highlight between saves/moves
- [ ] `persistentEditor` or an input dialog for `rename()`? (`openPersistentEditor(itemModel->indexFromItem(temp_item));`)
- [ ] Style scrollbars

### PaneDelegate
- [ ] For `paint()` and `updateEditorGeometry()` - these override public functions, and idk if that matters
- [ ] `updateEditorGeometry()` is currently unused

### Splitter

### Story

- [ ] Only hold X amount of backups per file

## Namespaces

### Index

### Io

### Path

- [ ] May not need `makePosix()`; Bit7z only accepts `\\` paths for searching

### Res

- [ ] There surely must be a smarter way to incorporate `.otf` into the fonts RC list (applies to MainWindow, too)

### Sample

### Uni

### Ud