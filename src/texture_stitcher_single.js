(function () {
    var button;
    var createRect = (x, y, w, h) => {
        return {x, y, w, h};
    };

    function packBoxes(boxes, padding) {
        var areas = [];
        var w = getInitialWidth(boxes, padding);
        var finalW = 0;
        var finalH = 0;

        boxes.sort((a, b) => b.h - a.h);
        areas.push(createRect(0, 0, w, Number.MAX_VALUE));

        boxes.forEach((box) => {
            for (var i = areas.length - 1; i >= 0; i--) {
                var area = areas[i];

                if (box.w > area.w || box.h > area.h) {
                    continue;
                }

                box.x = area.x;
                box.y = area.y;

                finalH = Math.max(finalH, box.y + box.h);
                finalW = Math.max(finalW, box.x + box.w);

                if (box.w == area.w && box.h == area.h) {
                    var last = areas.pop();

                    if (i < areas.length) {
                        areas.set(i, last);
                    }
                } else if (box.h == area.h) {
                    area.x += box.w;
                    area.w -= box.w;

                } else if (box.w == area.w) {
                    area.y += box.h;
                    area.h -= box.h;

                } else {
                    areas.push(createRect(area.x + box.w, area.y, area.w - box.w, box.h));

                    area.y += box.h;
                    area.h -= box.h;
                }

                break;
            }
        });

        /* Remove padding from boxes and add them to the final area */
        if (padding != 0) {
            boxes.forEach(glyph => {
                glyph.w -= padding;
                glyph.h -= padding;
                glyph.x += padding;
                glyph.y += padding;
            });

            finalW += padding;
            finalH += padding;
        }

        return [finalW, finalH];
    }

    function getInitialWidth(glyphs, padding) {
        var totalArea = 0;
        var maxW = 0;

        glyphs.forEach(box => {
            box.w += padding;
            box.h += padding;

            totalArea += box.w * box.h;
            maxW = Math.max(maxW, box.w);
        });

        return Math.max(Math.ceil(Math.sqrt(totalArea)), maxW);
    }

    function calculateRects(userPadding) {
        var rects = [];
        var maxScale = 1;
        var minScale = 1;
        var resize = false;

        Cube.selected.forEach(c => {
            if (c.box_uv) {
                resize = true;
            }
        });

        Texture.all.forEach(texture => {
            // Include any texture used by a selected cube
            let used = Cube.selected.some(cube =>
                Object.values(cube.faces).some(face => face.texture === texture.uuid)
            );
            if (texture.selected || texture.multi_selected || used) {

                maxScale = Math.max(maxScale, texture.width / texture.uv_width);
                minScale = Math.min(minScale, texture.width / texture.uv_width);
            }
        });

        maxScale /= minScale;
        minScale = 1;

        Texture.all.forEach(texture => {
            // Include any texture used by a selected cube
            let used = Cube.selected.some(cube =>
                Object.values(cube.faces).some(face => face.texture === texture.uuid)
            );
            if (texture.selected || texture.multi_selected || used) {

                var uv_scale = texture.width / texture.uv_width;
                var w = texture.width;
                var h = texture.height;
                var scale = resize ? 1 / uv_scale * maxScale : 1;
                var rect = createRect(0, 0, w * scale, h * scale);

                rect.texture = texture;
                rect.scale = scale;
                rect.uv_scale = uv_scale;

                rects.push(rect);
            }
        });

        var padding = userPadding * (resize ? maxScale : 1);
        var size = packBoxes(rects, padding);

        return {
            w: size[0],
            h: size[1],
            rects: rects,
            max_scale: maxScale,
            resize: resize
        };
    }

    function stitchSingleTextures(userPadding) {
        const data = calculateRects(userPadding);
        const rects = data.rects;

        const offscreen = new OffscreenCanvas(data.w, data.h);
        const c = offscreen.getContext('2d');

        /* Turn off AA */
        c.imageSmoothingEnabled = false;

        rects.forEach(rect => drawToCanvas(c, rect));

        const config = {
            type: 'image/png'
        };

        offscreen.convertToBlob(config).then(blob => {
            var reader = new FileReader();

            reader.readAsDataURL(blob);
            reader.onloadend = () => replaceTextures(data, reader.result);
        });
    }

    function drawToCanvas(c, rect) {
        c.drawImage(rect.texture.img, rect.x, rect.y, rect.w, rect.h)
    }

    function replaceTextures(data, imageData) {
        var maxScale = data.max_scale, resize = data.resize;
        var rects = data.rects, w = data.w, h = data.h;

        if (resize) {
            w /= maxScale;
            h /= maxScale;
        }

        const getRect = texture_uuid => {
            for (var i = 0; i < rects.length; i++) {
                if (rects[i].texture.uuid === texture_uuid) {
                    return rects[i];
                }
            }

            return null;
        };

        var sides = ['north', 'east', 'south', 'west', 'up', 'down']
        var texture = new Texture({
            mode: 'bitmap',
            name: 'stiched_texture',
            keep_size: true
        });

        var ts = [];

        Texture.all.forEach(texture => {
            // Include any texture used by a selected cube
            let used = Cube.selected.some(cube =>
                Object.values(cube.faces).some(face => face.texture === texture.uuid)
            );
            if (texture.selected || texture.multi_selected || used) {
                ts.push(texture)
            }
        });

        var elems = Cube.selected;

        if (Mesh) {
            elems = elems.concat(Mesh.selected);
        }

        Undo.initEdit({
            elements: elems,
            textures: Texture.all,
            bitmap: true,
            uv_mode: true
        });

        var newTextures = [];
        ts.forEach(t => newTextures.push(t));
        newTextures.forEach(t => t.remove(true));

        texture.fromDataURL(imageData).add(false).select();
        texture.uv_width = w;
        texture.uv_height = h;

        /* Required to work in Java Block/Item */
        Project.texture_width = w;
        Project.texture_height = h;

        Cube.selected.forEach(cube => {
            var toApplySides = [];

            if (cube.box_uv) {
                var north = cube.faces['north'];
                var rect = getRect(north.texture);

                if (rect != null) {
                    cube.uv_offset[0] += rect.x / maxScale;
                    cube.uv_offset[1] += rect.y / maxScale;
                }

                toApplySides = north.texture !== false;
            } else {
                sides.forEach(side => {
                    var face = cube.faces[side];
                    var rect = getRect(face.texture);

                    if (rect !== null) {
                        var mx = resize ? 1 : rect.uv_scale;
                        var my = resize ? 1 : rect.uv_scale;

                        face.uv[0] = face.uv[0] * mx + rect.x / (resize ? maxScale : 1);
                        face.uv[1] = face.uv[1] * my + rect.y / (resize ? maxScale : 1);
                        face.uv[2] = face.uv[2] * mx + rect.x / (resize ? maxScale : 1);
                        face.uv[3] = face.uv[3] * my + rect.y / (resize ? maxScale : 1);

                        if (face.texture !== false) {
                            toApplySides.push(side);
                        }
                    }
                });
            }

            if (toApplySides !== false) {
                cube.applyTexture(texture, toApplySides);
            }
        });

        if (Mesh) {
            Mesh.selected.forEach(mesh => {
                var applied = false;

                Object.keys(mesh.faces).forEach(key => {
                    var face = mesh.faces[key];
                    var rect = getRect(face.texture);

                    if (!rect) {
                        return;
                    }

                    Object.keys(face.uv).forEach(key => {
                        var uv = face.uv[key];
                        var mx = resize ? 1 : rect.uv_scale;
                        var my = resize ? 1 : rect.uv_scale;

                        uv[0] = uv[0] * mx + rect.x / (resize ? maxScale : 1);
                        uv[1] = uv[1] * my + rect.y / (resize ? maxScale : 1);

                        applied = true;
                    });
                });

                if (applied) {
                    mesh.applyTexture(texture, true);
                }
            });
        }

        Undo.finishEdit('finished stitching');
    }

    var stitchSingleDialog = new Dialog({
        id: "texture_stitcher_stitchSingle",
        title: "Stitch single textures",
        form: {
            padding: {
                label: "Padding",
                type: "number",
                min: 0,
                max: 12,
                step: 1,
                value: 1
            }
        },
        onConfirm(formData) {
            this.hide();

            stitchSingleTextures(formData.padding >= 0 ? formData.padding : 0)
        }
    });

    Plugin.register('texture_stitcher_single', {
        title: 'Texture Stitcher single',
        icon: 'fa-compress-arrows-alt',
        author: 'McHorse and Daniel99j',
        description: 'Stitch any textures into a single texture',
        version: '1.0.7',
        min_version: "4.8.0",
        tags: ["Texture"],
        variant: 'both',
        has_changelog: true,
        onload() {
            button = new Action('texture_stitcher_single', {
                name: 'Stitch selected textures',
                category: 'tools',
                description: 'Stitch selected textures into single texture (you might want to make a back up of the project)',
                icon: 'fa-compress-arrows-alt',
                click() {
                    stitchSingleDialog.show();
                }
            });

            MenuBar.addAction(button, 'tools');
        },
        onunload() {
            button.delete();
        }
    });
})();