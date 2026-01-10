# Translation

One way to contribute is translating the emulator. If you know other language, your contribution will be appreciated. It is more important the quality than the quantity. For this reason, AI translations will not be accepted.

This process explained in this guide could be lightly different because this project is in developement. For example, if exists a refactor of the code. For now, I indicate how to do that as simply as possible.

This interface is originally written in Spanish. That is the reason that English is the first language that is translated. Inspired by the translation to English, I contributed translating the interface to Catalan, which is one of my naive languages. I omit the explanation of Git and GitHub workflow. It is only focused on the translation.

## How I have done it

1. ### Create the file with the new translation.

The foreign languages can be found in the ```resources/i18n/``` folder. When I have done this, only exists a file:

```
trinity_en.ts
```

Copy the file and paste in the same folder putting a different name. The name must have the same prefix and the code of the language. If you do not know what it is, check [this page](https://www.andiamo.co.uk/resources/iso-language-codes/). If you do not need to indicate an specific country, use the standard code for this language.

In my case, Catalan has the code *ca*. The name of file is ```trinity_ca.ts```.
  
Now, in the ```resources/i18n/``` folder I have two files:

```
trinity_ca.ts  trinity_en.ts
```


2. ### Complete the translation.

The renamed file is a document in XML format. The only you need to do is completing the content between the ```<translation>``` and ```</translation>``` tag. Between the ```<source>``` and ```</source>``` tags, there is the text in Spanish, the original language of the interface. If you do not know Spanish, you can complete the ```<translation>``` fields using the ```<translation>``` tags of ```trinity_en.ts``` as the basis.

It is important to...

- NOT delete the ```%1```, ```%2```, ... symbols on the translations. This symbols prints parameters that are known in real time executing the code (a path, for example).
- Respect the spaces and line fields. It is important to respect format.

If you need to write the apostrophe (```'```), use this keyword: ```&apos;```. Do not put additional spaces at the beginning and the end.

#### Example in ```trinity_en.ts``` (lines 23-33):

```xml
    <message>
        <location filename="../../src/TrinityLib/core/exporter.cpp" line="36"/>
        <source>¿Exportar &apos;%1&apos; con datos del APK?

Sí: Incluye la versión completa (datos del juego).
No: Solo exporta mods, mapas, etc.</source>
        <translation>Export &apos;%1&apos; with APK data?

Yes: Includes full version (game data).
No: Only exports mods, maps, etc.</translation>
    </message>
```

#### The same example in ```trinity_ca.ts``` (lines 23-33):

```xml
    <message>
        <location filename="../../src/TrinityLib/core/exporter.cpp" line="36"/>
        <source>¿Exportar &apos;%1&apos; con datos del APK?

Sí: Incluye la versión completa (datos del juego).
No: Solo exporta mods, mapas, etc.</source>
        <translation>Exportar &apos;%1&apos; amb les dades de l&apos;APK?

Sí: Inclou versió completa (dades del joc).
No: Només exporta els mods, mapes, etc.</translation>
    </message>
```

3. ### Make the new file can be found during compilation

Only you need to add the code of your language (in my case, *ca*) on the ```CMakeLists.txt```.

Search this code and add the code in this line.

```cmake
# We define which language use
qt_standard_project_setup(I18N_TRANSLATED_LANGUAGES en ca)
```

4. ### Add the option  to choose this language in the interface

The launcher has a widget to change easily the language. To add the new language, you need to add a line on the ```src/TrinityLib/ui/windows/launcher_window.cpp``` file.

```c++
// --- Switcher languages
languageCombo = new QComboBox();
languageCombo->addItem("Español", "es"); // Index 0, data "es"
languageCombo->addItem("English", "en"); // Index 1, data "en"
languageCombo->addItem("Català", "ca"); // Index 2, data "ca"  <------- New line.
languageCombo->setFixedWidth(120);
```

If you choose ```Català``` in the UI, the ```Trinity Luncher.conf``` contains the correct setting:

```conf
[General]
language=ca
```

5. ### Link the language with the translation file

Inside the ```app/src``` folder, there are two files that will be modified:

```
main.cpp  trinito_main.cpp
```

We need to do the same change in both files:

```c++
QString lang = settings.value("language", "es").toString();

    if (lang == "en") {
        if (translator.load(":/i18n/trinity_en")) {
            app.installTranslator(&translator);
        }
    }
    // New code to add.
    else if (lang == "ca") {
        if (translator.load(":/i18n/trinity_ca")) {
            app.installTranslator(&translator);
        }
    }
```

6. ### Mission completed!

To see the result, follow the building instructions.
  
Remember that every time the language is changed, it is necessary to close and open the launcher again.
