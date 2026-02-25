import xml.etree.ElementTree as ET

ca_translations = {
    'Configuring': 'Configurant',
    'Trinity Launcher is an open-source, community-driven launcher for Minecraft Bedrock. Focused on user freedom and free redistribution, it provides a powerful interface to manage multiple instances, worlds, textures, and mods seamlessly.': 'Trinity Launcher és un llançador de codi obert impulsat per la comunitat per a Minecraft Bedrock. Enfocat en la llibertat de l\'usuari i la lliure distribució, proporciona una interfície potent per gestionar múltiples instàncies, mons, textures i mods sense problemes.',
    'Delete': 'Eliminar',
    'Trinity is built by a dedicated group of developers, designers, and contributors:': 'Trinity està construït per un grup dedicat de desenvolupadors, dissenyadors i col·laboradors:',
    'PLAY': 'JUGAR',
    'Join our community on Discord': 'Uneix-te a la nostra comunitat a Discord',
    '<b>Ezequiel</b>: Web Design & Frontend Developer.': '<b>Ezequiel</b>: Disseny Web i Desenvolupador Frontend.',
    'Shaders/Libs': 'Shaders / Llibreries',
    "¿Estás seguro de eliminar el mundo '%1'?\nEsta acción no se puede deshacer.": "Estàs segur d'eliminar el món '%1'?\nAquesta acció no es pot desfer.",
    'Menú de Trinity Launcher': 'Menú de Trinity Launcher',
    'Trinity Launcher': 'Trinity Launcher',
    'El juego ya se está ejecutando.': "El joc ja s'està executant.",
    '<b>JavierC</b>: Co-Creator & Development Supervisor.': '<b>JavierC</b>: Cocreador i Supervisor de Desenvolupament.',
    'Jugando Minecraft Bedrock': 'Jugant a Minecraft Bedrock',
    'Special Thanks': 'Agraïments Especials',
    '<b>KevinRunforrestt</b>: Documentation, Translation & Support.': '<b>KevinRunforrestt</b>: Documentació, Traducció i Suport.',
    '<b>HylianSoul</b>: Catalan Translation & Community Support.': '<b>HylianSoul</b>: Traducció al català i Suport Comunitari.',
    'Esperando...': 'Esperant...',
    'No hay ningún versión seleccionada.': 'No hi ha cap versió seleccionada.',
    'axe_icon': 'axe_icon',
    'Our Team': 'El Nostre Equip',
    'Cierra y vuelve a abrir el launcher para que se aplique la configuración.': "Tanca i torna a obrir el llançador per aplicar la configuració.",
    'We would like to express our sincere gratitude to the team behind the <b>Unofficial NIX Launcher for Minecraft</b>. Their work provides the essential runtime to run Minecraft, which has been fundamental to the development of this project.': "Ens agradaria expressar el nostre sincer agraïment a l'equip de l'<b>Unofficial NIX Launcher for Minecraft</b>. El seu treball proporciona l'entorn d'execució essencial per executar Minecraft, la qual cosa ha estat fonamental per al desenvolupament d'aquest projecte.",
    'Resource Pack': 'Paquet de Recursos',
    'Bedrock Edition': 'Edició Bedrock',
    '<b>IoselDev</b>: AUR Package Maintainer.': '<b>IoselDev</b>: Mantenidor del paquet AUR.',
    'No se pudo iniciar el proceso del juego.': "No s'ha pogut iniciar el procés del joc.",
    'Trinity': 'Trinity',
    '+ Extract APK': '+ Extreure APK',
    'Content Manager': 'Gestor de Continguts',
    'About Trinity Launcher': 'Sobre Trinity Launcher',
    'Select a version': 'Selecciona una versió',
    'mcpelauncher-client no encontrado.': "No s'ha trobat mcpelauncher-client.",
    'Discord Rich Presence': 'Activitat de Discord',
    'Discord': 'Discord',
    'Edit Config': 'Editar Configuració',
    'Create Shortcut': 'Crear Accés Directe',
    'Waiting to start': 'Esperant per iniciar',
    'Ready': 'A punt',
    'Trinity Launcher - Minecraft Bedrock': 'Trinity Launcher - Minecraft Bedrock',
    'Import': 'Importar',
    'Mods': 'Mods',
    'Export': 'Exportar',
    'Development Packs': 'Paquets de Desenvolupament',
    'In the main menu': 'Al menú principal',
    'Behavior Pack (mods)': 'Paquet de Comportament (mods)',
    '<b>Future Contributor</b>: This spot is reserved for you. Join us!': "<b>Futur Col·laborador</b>: Aquest espai està reservat per a tu. Uneix-te!",
    '<b>MrTanuk</b>: Core Developer.': '<b>MrTanuk</b>: Desenvolupador Principal.',
    'Join Discord': 'Entrar a Discord',
    '<b>Crow</b>: Project Creator & Visionary.': '<b>Crow</b>: Creador del Projecte i Visionari.',
    '<b>Orta</b>: Project Supervisor & Software Architect.': '<b>Orta</b>: Supervisor del Projecte i Arquitecte de Programari.'
}

uk_translations = {
    'Configuring': 'Налаштування',
    'Trinity Launcher is an open-source, community-driven launcher for Minecraft Bedrock. Focused on user freedom and free redistribution, it provides a powerful interface to manage multiple instances, worlds, textures, and mods seamlessly.': 'Trinity Launcher — це лаунчер з відкритим вихідним кодом для Minecraft Bedrock, створений спільнотою. Орієнтований на свободу користувача та вільне поширення, він забезпечує потужний інтерфейс для зручного керування кількома екземплярами, світами, текстурами та модами.',
    'Delete': 'Видалити',
    'Trinity is built by a dedicated group of developers, designers, and contributors:': 'Trinity створено відданою групою розробників, дизайнерів та учасників:',
    'PLAY': 'ГРАТИ',
    'Join our community on Discord': 'Приєднуйтесь до нашої спільноти у Discord',
    '<b>Ezequiel</b>: Web Design & Frontend Developer.': '<b>Ezequiel</b>: Веб-дизайн та Frontend-розробник.',
    'Shaders/Libs': 'Шейдери/Бібліотеки',
    "¿Estás seguro de eliminar el mundo '%1'?\nEsta acción no se puede deshacer.": "Ви впевнені, що хочете видалити світ '%1'?\nЦю дію неможливо скасувати.",
    'Menú de Trinity Launcher': 'Меню Trinity Launcher',
    'Trinity Launcher': 'Trinity Launcher',
    'El juego ya se está ejecutando.': 'Гра вже запущена.',
    '<b>JavierC</b>: Co-Creator & Development Supervisor.': '<b>JavierC</b>: Співтворець та керівник розробки.',
    'Jugando Minecraft Bedrock': 'Грає в Minecraft Bedrock',
    'Special Thanks': 'Особлива подяка',
    '<b>KevinRunforrestt</b>: Documentation, Translation & Support.': '<b>KevinRunforrestt</b>: Документація, переклад та підтримка.',
    '<b>HylianSoul</b>: Catalan Translation & Community Support.': '<b>HylianSoul</b>: Переклад на каталонську та підтримка спільноти.',
    'Esperando...': 'Очікування...',
    'No hay ningún versión seleccionada.': 'Жодної версії не вибрано.',
    'axe_icon': 'axe_icon',
    'Our Team': 'Наша команда',
    'Cierra y vuelve a abrir el launcher para que se aplique la configuración.': 'Закрийте та знову відкрийте лаунчер, щоб застосувати налаштування.',
    'We would like to express our sincere gratitude to the team behind the <b>Unofficial NIX Launcher for Minecraft</b>. Their work provides the essential runtime to run Minecraft, which has been fundamental to the development of this project.': 'Ми хотіли б висловити нашу щиру подяку команді <b>Unofficial NIX Launcher for Minecraft</b>. Їхня робота забезпечує необхідне середовище для запуску Minecraft, що стало фундаментальним для розвитку цього проєкту.',
    'Resource Pack': 'Пакет ресурсів',
    'Bedrock Edition': 'Bedrock Edition',
    '<b>IoselDev</b>: AUR Package Maintainer.': '<b>IoselDev</b>: Супроводжувач пакета AUR.',
    'No se pudo iniciar el proceso del juego.': 'Не вдалося запустити ігровий процес.',
    'Trinity': 'Trinity',
    '+ Extract APK': '+ Видобути APK',
    'Content Manager': 'Менеджер контенту',
    'About Trinity Launcher': 'Про Trinity Launcher',
    'Select a version': 'Виберіть версію',
    'mcpelauncher-client no encontrado.': 'mcpelauncher-client не знайдено.',
    'Discord Rich Presence': 'Розширена присутність Discord',
    'Discord': 'Discord',
    'Edit Config': 'Редагувати конфігурацію',
    'Create Shortcut': 'Створити ярлик',
    'Waiting to start': 'Очікування запуску',
    'Ready': 'Готово',
    'Trinity Launcher - Minecraft Bedrock': 'Trinity Launcher - Minecraft Bedrock',
    'Import': 'Імпортувати',
    'Mods': 'Моди',
    'Export': 'Експортувати',
    'Development Packs': 'Пакети розробника',
    'In the main menu': 'У головному меню',
    'Behavior Pack (mods)': 'Пакет поведінки (моди)',
    '<b>Future Contributor</b>: This spot is reserved for you. Join us!': '<b>Майбутній учасник</b>: Це місце зарезервовано для вас. Приєднуйтесь!',
    '<b>MrTanuk</b>: Core Developer.': '<b>MrTanuk</b>: Головний розробник.',
    'Join Discord': 'Приєднатися до Discord',
    '<b>Crow</b>: Project Creator & Visionary.': '<b>Crow</b>: Творець проєкту та візіонер.',
    '<b>Orta</b>: Project Supervisor & Software Architect.': '<b>Orta</b>: Керівник проєкту та архітектор програмного забезпечення.'
}

pt_translations = {
    'Configuring': 'Configurando',
    'Trinity Launcher is an open-source, community-driven launcher for Minecraft Bedrock. Focused on user freedom and free redistribution, it provides a powerful interface to manage multiple instances, worlds, textures, and mods seamlessly.': 'O Trinity Launcher é um inicializador de código aberto e movido pela comunidade para Minecraft Bedrock. Focado na liberdade do usuário e na redistribuição gratuita, ele fornece uma interface poderosa para gerenciar múltiplas instâncias, mundos, texturas e mods de forma integrada.',
    'Delete': 'Excluir',
    'Trinity is built by a dedicated group of developers, designers, and contributors:': 'O Trinity é construído por um grupo dedicado de desenvolvedores, designers e colaboradores:',
    'PLAY': 'JOGAR',
    'Join our community on Discord': 'Junte-se à nossa comunidade no Discord',
    '<b>Ezequiel</b>: Web Design & Frontend Developer.': '<b>Ezequiel</b>: Web Design e Desenvolvedor Frontend.',
    'Shaders/Libs': 'Shaders / Bibliotecas',
    "¿Estás seguro de eliminar el mundo '%1'?\nEsta acción no se puede deshacer.": "Tem certeza de que deseja excluir o mundo '%1'?\nEsta ação não pode ser desfeita.",
    'Menú de Trinity Launcher': 'Menu do Trinity Launcher',
    'Trinity Launcher': 'Trinity Launcher',
    'El juego ya se está ejecutando.': "O jogo já está em execução.",
    '<b>JavierC</b>: Co-Creator & Development Supervisor.': '<b>JavierC</b>: Cocriador e Supervisor de Desenvolvimento.',
    'Jugando Minecraft Bedrock': 'Jogando Minecraft Bedrock',
    'Special Thanks': 'Agradecimentos Especiais',
    '<b>KevinRunforrestt</b>: Documentation, Translation & Support.': '<b>KevinRunforrestt</b>: Documentação, Tradução e Suporte.',
    '<b>HylianSoul</b>: Catalan Translation & Community Support.': '<b>HylianSoul</b>: Tradução para o Catalão e Suporte da Comunidade.',
    'Esperando...': 'Aguardando...',
    'No hay ningún versión seleccionada.': 'Não há nenhuma versão selecionada.',
    'axe_icon': 'axe_icon',
    'Our Team': 'Nossa Equipe',
    'Cierra y vuelve a abrir el launcher para que se aplique la configuración.': "Feche e reabra o inicializador para aplicar as configurações.",
    'We would like to express our sincere gratitude to the team behind the <b>Unofficial NIX Launcher for Minecraft</b>. Their work provides the essential runtime to run Minecraft, which has been fundamental to the development of this project.': "Gostaríamos de expressar nossa sincera gratidão à equipe responsável pelo <b>Unofficial NIX Launcher for Minecraft</b>. O trabalho deles fornece o ambiente essencial para executar o Minecraft, o que tem sido fundamental para o desenvolvimento deste projeto.",
    'Resource Pack': 'Pacote de Recursos',
    'Bedrock Edition': 'Edição Bedrock',
    '<b>IoselDev</b>: AUR Package Maintainer.': '<b>IoselDev</b>: Mantenedor do pacote AUR.',
    'No se pudo iniciar el proceso del juego.': "Não foi possível iniciar o processo do jogo.",
    'Trinity': 'Trinity',
    '+ Extract APK': '+ Extrair APK',
    'Content Manager': 'Gerenciador de Conteúdo',
    'About Trinity Launcher': 'Sobre o Trinity Launcher',
    'Select a version': 'Selecione uma versão',
    'mcpelauncher-client no encontrado.': "mcpelauncher-client não foi encontrado.",
    'Discord Rich Presence': 'Status do Discord',
    'Discord': 'Discord',
    'Edit Config': 'Editar Configurações',
    'Create Shortcut': 'Criar Atalho',
    'Waiting to start': 'Aguardando início',
    'Ready': 'Pronto',
    'Trinity Launcher - Minecraft Bedrock': 'Trinity Launcher - Minecraft Bedrock',
    'Import': 'Importar',
    'Mods': 'Mods',
    'Export': 'Exportar',
    'Development Packs': 'Pacotes de Desenvolvimento',
    'In the main menu': 'No menu principal',
    'Behavior Pack (mods)': 'Pacote de Comportamento (mods)',
    '<b>Future Contributor</b>: This spot is reserved for you. Join us!': "<b>Futuro Colaborador</b>: Este espaço é reservado para você. Junte-se a nós!",
    '<b>MrTanuk</b>: Core Developer.': '<b>MrTanuk</b>: Desenvolvedor Principal.',
    'Join Discord': 'Entrar no Discord',
    '<b>Crow</b>: Project Creator & Visionary.': '<b>Crow</b>: Criador do Projeto e Visionário.',
    '<b>Orta</b>: Project Supervisor & Software Architect.': '<b>Orta</b>: Supervisor do Projeto e Arquiteto de Software.'
}

def apply_translations(filename, translations_dict):
    tree = ET.parse(filename)
    root = tree.getroot()
    changed = False
    
    for message in root.iter('message'):
        trans = message.find('translation')
        if trans is not None and trans.get('type') == 'unfinished':
            source = message.find('source')
            if source is not None and source.text:
                if source.text in translations_dict:
                    trans.text = translations_dict[source.text]
                    del trans.attrib['type']
                    changed = True
                else:
                    print(f"Missing in script: '{source.text}'")

    if changed:
        tree.write(filename, encoding='utf-8', xml_declaration=True)
        print(f"Updated {filename}")

apply_translations('resources/i18n/trinity_ca.ts', ca_translations)
apply_translations('resources/i18n/trinity_uk.ts', uk_translations)
apply_translations('resources/i18n/trinity_pt.ts', pt_translations)
