//edited version of https://github.com/sciner/webcraft-blockbench-plugin/blob/main/madcraft_blockbench_plugin.js
(function () {

    const property_name = 'baseplate'
    const removables = []
    const deletables = []
    const default_value = { flags: [], json: null, shader: 'regular' }
    const default_value_string = JSON.stringify(default_value)
    const baseplate_css = `
    .baseplate-widget {
        position: relative;
        margin: 4px;
        width: 100%;
    }
    #highlighting {
        position: absolute;
        top: 0;
        left: 0;
        margin: 0;
        padding: 4px;
        width: 100%;
        pointer-events: none;
        background: transparent;
    }
    textarea.input-property {
        background-color: var(--color-back);
        color: transparent;
        caret-color: white;
        padding: 4px;
    }
    textarea.input-property:read-only {
        opacity: .25;
    }
    textarea.input-property, #highlighting-content {
        display: block;
        width: 100%;
        height: 90px;
        font-family: var(--font-code);
        font-size: 14px;
    }
    .baseplate-tag {
        padding: 3px 8px;
        border-radius: 2px;
        background-color: var(--color-button);
        display: inline-block;
        margin: 2px;
        line-height: 1em;
    }
    .baseplate-tag:hover {
        background-color: rgba(255, 255, 255, .35);
    }
    .select_control_box {
        padding: 4px;
    }
    .baseplate-delete-tag {
        border-radius: 50px;
        width: 16px;
        height: 16px;
        margin-left: .25em;
        background-color: rgba(0, 0, 0, .2);
        display: inline-flex;
        align-items: center;
        justify-content: center;
        cursor: pointer;
    }
    .baseplate-delete-tag:hover {
        background-color: var(--color-close);
    }
    .baseplate-select {
        width: 100%;
        border-radius: 4px;
        margin-top: 2px;
    }
    `

    let codec;

    class BasePlatePropertyEditWidget extends Widget {
        controls = []

        constructor(id, data) {
            if (typeof id === 'object') {
                data = id
                id = data.id
            }
            super(id, data)
            this.property_name = data.property_name
            this.value_name = data.value_name
        }

        cloneObject(object) {
            return JSON.parse(JSON.stringify(object))
        }

        getProps() {
            if (this.props) {
                return this.props
            }

            if (!this.elements || this.elements.length === 0) {
                return null
            }

            const first = this.elements[0][this.property_name] ?? this.cloneObject(default_value)

            let allSame = true
            for (let i = 1; i < this.elements.length; i++) {
                const current = this.elements[i][this.property_name] ?? default_value
                if (JSON.stringify(current) !== JSON.stringify(first)) {
                    allSame = false
                    break
                }
            }

            const result = allSame ? this.cloneObject(first) : this.cloneObject(default_value)
            return this.props = result
        }

        setElementValue(name, value) {
            console.log("setting "+name+" to "+value);
            if (!this.elements || this.elements.length === 0) return

            const newProps = this.getProps()
            newProps[name] = value

            for (const element of this.elements) {
                const props = element[this.property_name] = this.cloneObject(newProps);
                if (JSON.stringify(props) === default_value_string) {
                    element[this.property_name] = null
                }
            }

            this.props = newProps
            Undo.initEdit({elements: []}); Undo.finishEdit('edit baseplate properties', {elements: this.elements});
        }

        update() {
            this.updateProps();
            for (const control of this.controls) {
                control.update()
            }
        }

        updateProps() {
            const faces = []
            var useNormal = true;
            console.log("updating");
            if (Mesh.selected && Mesh.selected.length > 0) {
                for (const mesh1 of Mesh.selected) {
                    if (mesh1.getSelectedFaces().length > 0) {
                        useNormal = false;
                        for (const face1 of mesh1.getSelectedFaces()) {
                            faces.push(mesh1.faces[face1]);
                        }
                    }
                }
            }
            if (useNormal) for (const sel of Outliner.selected) {
                if (sel instanceof Mesh) {
                    sel.forAllFaces(face => faces.push(face))
                }
            }

            this.elements = faces
            this.props = null // Clear cache so getProps recomputes
            this.getProps();
        }

        createWidgetNode() {
            const nodes = []
            for (const control of this.controls) {
                nodes.push(...control.nodes)
            }
            this.node = Interface.createElement('div', { class: 'widget baseplate-widget' }, nodes)
        }
    }


    // Edit JSON
    class BasePlateJSONWidget extends BasePlatePropertyEditWidget {

        constructor(id, data) {
            super(id, data)
            this.type = 'jsonedit'
            this.controls.push(this.createJSONControl(this.value_name))
            this.createWidgetNode()
        }

        createJSONControl(value_name, options) {
            this.updateProps();

            const code = this.jq_code = Interface.createElement('code', { class: 'language-json', id: 'highlighting-content' })
            const textarea = this.jq_textarea = Interface.createElement('textarea', { class: 'dark_bordered focusable_input input-property' })
            textarea.addEventListener('input', () => {
                if (this.elements) {
                    const text = textarea.value
                    try {
                        const json = text.trim().length > 0 ? JSON.parse(text) : null
                        this.setElementValue(value_name, json)
                    } catch (e) {
                        // do nothing
                    }
                    this.updateJSON(text)
                }
            })

            const nodes = [
                textarea,
                Interface.createElement('pre', { id: 'highlighting', 'aria-hidden': true }, [
                    code
                ])
            ]

            return {
                update: () => {
                    const elements = this.elements
                    const text = elements ? ((this.props && this.props[value_name]) ? JSON.stringify(this.props[value_name]) : '') : ''
                    this.updateJSON(text)
                }, nodes
            }

        }

        updateJSON(text) {
            this.updateProps();
            const ta = this.jq_textarea
            ta.value = text
            const code = this.jq_code
            code.textContent = text
            Prism.highlightElement(code)
            ta.readOnly = !this.elements
        }

    }

    // Editor multiselect
    class BasePlateMultiselectWidget extends BasePlatePropertyEditWidget {

        constructor(id, data) {
            super(id, data)
            this.type = 'flagsedit'
            this.controls.push(this.createMultiSelectPropertyControl(this.value_name, data.options))
            this.createWidgetNode()
        }

        createMultiSelectPropertyControl(property_name, options) {
            this.updateProps();

            const div_flags = Interface.createElement('div', {})
            const select = Interface.createElement('select', { class: 'baseplate-select' })
            const select_control_box = Interface.createElement('div', { class: 'select_control_box' }, [div_flags, select])

            const redrawSelect = () => {
                const props = this.getProps()
                const options_html = []
                options_html.push(`<option value="">Add...</option>`)
                for (const [title, value] of Object.entries(options)) {
                    if (!props || !props[this.value_name].includes(value)) {
                        options_html.push(`<option value="${value}">${title}</option>`)
                    }
                }
                if (props) {
                    div_flags.innerHTML = ''
                    for (const value of props[this.value_name]) {
                        let title = value
                        for (let k in options) {
                            if (options[k] == value) {
                                title = k
                                break
                            }
                        }
                        const delete_tag_button = Interface.createElement('span', { class: 'baseplate-delete-tag', 'data-value': value }, ['×'])
                        div_flags.appendChild(Interface.createElement('span', { class: 'baseplate-tag' }, [title, delete_tag_button]))
                        delete_tag_button.addEventListener('click', (e) => {
                            if (this.elements) {
                                const value = e.srcElement.dataset.value
                                if (value) {
                                    const data = this.getProps();
                                    if (data) {
                                        const list = data[this.value_name]
                                        const index = list.indexOf(value)
                                        if (index >= 0) {
                                            list.splice(index, 1)
                                            this.setElementValue(this.value_name, list);
                                            redrawSelect()
                                        }
                                    }
                                }
                            }
                        })
                    }
                }
                select.innerHTML = options_html.join('\n')
            }

            //
            select.addEventListener('change', (e) => {
                if (this.elements) {
                    const value = e.srcElement.value
                    if (value) {
                        const data = this.getProps()[this.value_name];
                        data.push(value);
                        this.setElementValue(this.value_name, data);
                        redrawSelect()
                    }
                }
            })

            redrawSelect()

            return { update: () => { redrawSelect() }, nodes: [select_control_box] }
        }

    }

    // Editor select
    class BasePlateSelectWidget extends BasePlatePropertyEditWidget {

        constructor(id, data) {
            super(id, data)
            this.type = 'selectedit'
            this.controls.push(this.createSelectPropertyControl(this.value_name, data.options))
            this.createWidgetNode()
        }

        createSelectPropertyControl(property_name, options) {
            this.update();

            const select = Interface.createElement('select', { class: 'baseplate-select' })
            const select_control_box = Interface.createElement('div', { class: 'select_control_box' }, [select])

            const redrawSelect = () => {
                const props = this.getProps()
                const options_html = []
                options_html.push(`<option value="">Select...</option>`)
                for (const [title, value] of Object.entries(options)) {
                    const selected = props ? (props[property_name] == value ? 'selected' : null) : null
                    options_html.push(`<option value="${value}" ${selected}>${title}</option>`)
                }
                select.innerHTML = options_html.join('\n')
            }

            //
            select.addEventListener('change', (e) => {
                if (this.elements) {
                    const value = e.srcElement.value
                    if (value) {
                        const props = this.getProps()
                        if (props) {
                            this.setElementValue(this.value_name, value);
                            // redrawSelect()
                        }
                    }
                }
            })

            redrawSelect()

            return { update: () => { redrawSelect() }, nodes: [select_control_box] }
        }

    }

    //
    class MyToolbar extends Toolbar {

        constructor(options) {
            super(options)
            this.condition = () => (selected.length && Modes.edit)
        }

        update() {
            super.update()
            for (const control of this.children) {
                control.update()
            }
        }

    }

    // Create toolbar
    function createToolbar(name, widgets) {
        const toolbar = new MyToolbar({
            id: `tb_${name}`.toLowerCase().replaceAll(' ', '_').trim(),
            name: name,
            label: true,
            children: widgets.map((w) => w.id)
        })
        Interface.Panels.element.addToolbar(toolbar)
        removables.push(toolbar)
    }

    // Init display
    function initDisplay() {
    }

    Plugin.register('baseplate_plugin', {
        title: 'Baseplate',
        author: 'Daniel99j',
        description: 'Adds the baseplate model format',
        icon: 'developer_mode',
        version: '1.0.0',
        variant: 'both',
        about: "Adds a bunch of stuff for my C++ engine",
        tags: ["Plugins", "BasePlate"],
        min_version: "4.7.4",

        onload() {
            codec = new Codec("bplate_codec", {
                name: "Baseplate Model",
                extension: "bplate",
                load_filter: {
                    extensions: ["bplate"],
                    type: "json"
                },
                remember: true,
                compile() {
                    const data = JSON.parse(Codecs.project.compile());

                    data.model_format ??= "free";
                    return autoStringify(data);
                },
                parse(content) {
                    Codecs.project.parse(content);
                }
            });
            codec.format = new ModelFormat({
                id: "bplate_format",
                name: "Baseplate model file",
                description: "My custom model format for my C++ engine",
                icon: "extension",
                category: "bplate",
                rotate_cubes: true,
                edit_mode: true,
                optional_box_uv: true,
                meshes: true,
                texture_meshes: true,
                texture_mcmeta: false,
                show_on_start_screen: true,
                java_cube_shading_properties: false,
                codec
            });

            deletables.push(codec);
            // Create new property for all Cubes
            deletables.push(new Property(MeshFace, 'instance', property_name, { default: null, exposed: true, label: "BasePlate custom properties" }))
            // CSS
            Blockbench.addCSS(baseplate_css)
            // Display
            initDisplay()
            // Create toolbars
            createToolbar('BasePlate JSON', [new BasePlateJSONWidget('baseplate_cube_json_widget', { property_name, value_name: 'json' })])
            createToolbar('BasePlate shader', [new BasePlateSelectWidget('baseplate_cube_shader_widget', {
                property_name, value_name: 'shader', options: {
                    'Regular': 'regular',
                    'Singleface': 'singleface',
                    'Doubleface': 'doubleface',
                    'Doubleface solid': 'doubleface_solid',
                    'Transparent': 'transparent',
                    'Doubleface + Transparent': 'doubleface_transparent',
                    'Decal 1': 'decal1',
                    'Decal 2': 'decal2',
                }
            })])
            createToolbar('BasePlate flags', [new BasePlateMultiselectWidget('baseplate_cube_flags_widget', {
                property_name, value_name: 'flags', options: {
                    'No collision': 'FLAG_NO_COLLISION',
                }
            })])

            // animations search

            const def_colors = {
                idle: '#DFCFBE',
                firstperson: '#D65076',
                emote: '#f4d90a',
                walk: '#009B77',
                sitting: '#88B04B',
                levitate: '#92A8D1',
                sneak: '#B565A7',
                sleep: '#EFC050',
                attack: '#E15D44',
            }

            class BasePlateProject {
                x
                constructor(id) {
                    this.id = id
                    this.groups = {}
                    this.save_key = `baseplate_project_${id}`
                    this.load()
                }

                load() {
                    let saved_data = localStorage.getItem(this.save_key)
                    saved_data = saved_data ? JSON.parse(saved_data) : null
                    if (saved_data) {
                        Object.assign(this, saved_data)
                    }
                }

                save() {
                    const saved_data = {
                        groups: this.groups
                    }
                    localStorage.setItem(this.save_key, JSON.stringify(saved_data))
                }

                getGroup(name) {
                    let group = this.groups[name]
                    if (!group) {
                        group = this.groups[name] = { folded: false }
                    }
                    return group
                }

            }

            class BasePlatePlugin {
                projects = new Map()
                div
                input
                last_query = ''
                anim_colors = {}
                dialog = null
                panel = Interface.Panels.animations
                colors_key = 'baseplate-anim_colors'

                constructor() {

                    this.anim_colors = this.loadColors()

                    this.createDialog()

                    this.panel.on('update', (args) => {
                        console.log('panel:on', args)
                        if (args.show) {
                            this.addSearchField()
                        }
                    })

                }

                // dialog
                createDialog() {
                    const that = this
                    let prop = JSON.stringify(this.anim_colors, null, 4)
                    this.dialog = new Dialog({
                        id: this.colors_key,
                        title: 'Enter JSON Colors',
                        form: {
                            custom_text: { label: 'Colors', type: 'textarea', value: prop },
                        },
                        onConfirm(form_result) {
                            prop = form_result.custom_text
                            that.saveColors(prop)
                        }
                    })
                }

                getProjectID() {
                    return Project.name
                }

                getProject() {
                    const id = this.getProjectID()
                    // return Project.baseplate_project
                    let project = this.projects.get(id)
                    if (project) {
                        return project
                    }
                    project = new BasePlateProject(id)
                    this.projects.set(project.id, project)
                    return project
                }

                loadColors() {
                    let str = localStorage.getItem(this.colors_key) || JSON.stringify(def_colors, null, 4)
                    let json = JSON.parse(str) || def_colors
                    return json
                }

                saveColors(colors) {
                    let json = JSON.parse(colors)
                    if (json) {
                        localStorage.setItem(this.colors_key, colors)
                        Object.assign(this.anim_colors, json)
                        this.filterAnimations(this.last_query)
                    }
                }

                toggleGroup(el, group_name, is_group) {
                    const { input } = this
                    el.style.backgroundColor = is_group ? '#ffffff22' : 'revert-layer'
                    // toggler
                    let toggler = el.querySelector('.icon-open-state')
                    if (!toggler) {
                        toggler = document.createElement('i')
                        toggler.classList.add('icon-open-state', 'fa', 'fa-angle-down')
                        toggler.style.display = 'flex'
                        toggler.style.alignItems = 'center'
                        toggler.onclick = () => {
                            const project = this.getProject()
                            const group = project.getGroup(group_name)
                            group.folded = !group.folded
                            project.save()
                            this.filterAnimations(input.value)
                        }
                        el.prepend(toggler)
                    }
                    toggler.style.display = is_group ? 'flex' : 'none'
                    // 
                    for (const child of el.children) {
                        const classes = child.classList
                        if (classes.contains('shader-icons') || classes.contains('in_list_button')) {
                            child.style.display = is_group ? 'none' : 'unset'
                        } else if (child.tagName == 'LABEL') {
                            child.style.textAlign = is_group ? 'center' : 'left'
                        }
                    }
                }

                filterAnimations(query) {
                    const lower = this.last_query = query.toLowerCase()
                    let group_name = '_root_'
                    const project = this.getProject()

                    Animator.animations.forEach(anim => {
                        const list_elem = this.panel.node.querySelector(`[anim_id="${anim.uuid}"]`)
                        if (!list_elem) {
                            console.log('no list_elem')
                            return
                        }
                        const name = anim.name.toLowerCase()
                        let color = '#ffffff22'
                        for (const [key, value] of Object.entries(this.anim_colors)) {
                            if (name.startsWith(key)) {
                                color = value
                                break
                            }
                        }
                        let is_group = name.includes('888') || name.includes('---') || name.includes('===')
                        if (is_group) {
                            group_name = name
                        }
                        const group_visible = !project.getGroup(group_name).folded
                        let is_visible = (name.includes(lower) && group_visible) || is_group
                        list_elem.style.borderLeft = `4px solid ${color}`
                        list_elem.style.display = is_visible ? 'revert-layer' : 'none'
                        this.toggleGroup(list_elem, group_name, is_group)
                    })
                }

                addSearchField() {
                    const anim_list = this.panel.node.querySelector('.toolbar')
                    if (!anim_list) return
                    let div = this.div
                    if (!div || !document.body.contains(div)) {
                        div = this.div = document.createElement('div')
                        div.style.display = 'flex'
                        div.style.flexDirection = 'row'
                        const input = this.input = document.createElement('input')
                        input.placeholder = 'Search animations...'
                        input.style.margin = '4px'
                        input.style.width = '100%'
                        input.style.paddingLeft = '.9em'
                        input.style.margin = '0px'
                        input.value = this.last_query
                        input.oninput = () => this.filterAnimations(input.value)
                        div.appendChild(input)
                        const button = document.createElement('button')
                        button.innerHTML = '<i class="shader-icons notranslate icon">tune</i>'
                        button.style.margin = '0px'
                        button.style.minWidth = 'auto'
                        button.onclick = () => this.dialog.show()
                        div.appendChild(button)
                    }
                    if (div.parentElement !== anim_list.parentElement) {
                        anim_list.appendChild(div)
                    }
                    this.filterAnimations(this.input.value)
                }

            }

            const baseplate_plugin = new BasePlatePlugin()

            Blockbench.on('load_project', data => {
                // Project.baseplate_project = data.model.baseplate_project || {
                //     groups: {}
                // }
                // Project.baseplate_project = data.model.baseplate_project || {
                //     groups: {}
                // }
                baseplate_plugin.addSearchField()
            })

            // Blockbench.on('save_project', (event) => {
            //     event.model.baseplate_project = Project.baseplate_project
            // })


        },

        onunload() {
            deletables.forEach(action => {
                action.delete();
            })
            removables.forEach(action => {
                action.remove();
            })
        }

    })

})()