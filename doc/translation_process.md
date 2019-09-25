Translations
============

The Binarium Core project has been designed to support multiple localisations.
This makes adding new phrases, and completely new languages easily achievable.

Multiple language support is critical in assisting Binarium’s global adoption,
and growth. One of Binarium’s greatest strengths is cross-boarder money transfers
 any help making that easier is greatly appreciated.

### Writing code with translations
We use automated scripts to help extract translations in both Qt, and non-Qt
source files. It is rarely necessary to manually edit the files in `src/qt/locale/`.
The translation source files must adhere to the following format:
`binarium_xx_YY.ts or binarium_xx.ts`

`src/qt/locale/binarium_en.ts` is treated in a special way. It is used as the
source for all other translations. Whenever a string in the source code is changed,
this file must be updated to reflect those changes. A custom script is used to
extract strings from the non-Qt parts. This script makes use of `gettext`, so
make sure that utility is installed (ie, `apt-get install gettext` on Ubuntu/Debian).
Once this has been updated, `lupdate` (included in the Qt SDK) is used to update
`binarium_en.ts`.

To automatically regenerate the `binarium_en.ts` file, run the following commands:
```sh
cd src/
make translate
```

`contrib/binarium-qt.pro` takes care of generating `.qm` (binary compiled) files
from `.ts` (source files) files. It’s mostly automated, and you shouldn’t need
to worry about it.

**Example Qt translation**
```cpp
QToolBar *toolbar = addToolBar(tr("Tabs toolbar"));
```

### Creating a pull-request
For general PRs, you shouldn’t include any updates to the translation source
files. They will be updated periodically, primarily around pre-releases,
allowing time for any new phrases to be translated before public releases. This
is also important in avoiding translation related merge conflicts.

To create the pull-request, use the following commands:
```
git add src/qt/binariumstrings.cpp src/qt/locale/binarium_en.ts
git commit
```


**For Windows**

### Synchronising translations
To assist in updating translations, we have created a script to help.

1. `python contrib/devtools/update-translations.py`
2. Update `src/qt/binarium_locale.qrc` manually or via
   `ls src/qt/locale/*ts|xargs -n1 basename|sed 's/\(binarium_\(.*\)\).ts/<file alias="\2">locale\/\1.qm<\/file>/'`
3. Update `src/Makefile.qt.include` manually or via
   `ls src/qt/locale/*ts|xargs -n1 basename|sed 's/\(binarium_\(.*\)\).ts/  qt\/locale\/\1.ts \\/'`
4. `git add` new translations from `src/qt/locale/`

### Handling Plurals (in source files)
When new plurals are added to the source file, it's important to do the following steps:

1. Open `binarium_en.ts` in Qt Linguist (included in the Qt SDK)
2. Search for `%n`, which will take you to the parts in the translation that use plurals
3. Look for empty `English Translation (Singular)` and `English Translation (Plural)` fields
4. Add the appropriate strings for the singular and plural form of the base string
5. Mark the item as done (via the green arrow symbol in the toolbar)
6. Repeat from step 2, until all singular and plural forms are in the source file
7. Save the source file

### Translating a new language
To create a new language template, you will need to edit the languages manifest
file `src/qt/binarium_locale.qrc` and add a new entry. Below is an example of
the English language entry.

```xml
<qresource prefix="/translations">
    <file alias="en">locale/binarium_en.qm</file>
    ...
</qresource>
```

**Notes**

That the language translation file **must end in `.qm`** (the compiled extension), and not `.ts`.
