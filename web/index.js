var MakerPage = PageView.extend(
    {
        pageTitle: function() {
            return this.model.get('maker_name');
        },
        events: {
            'click button[name=new-item]': 'showCreateDialog'
        },
        initialize: function() {
            PageView.prototype.initialize.apply(this, arguments);
            this.model.fetch();

            this.$el.html(this.template(this.templateParams()));

            this._detailsView = new StaticView({
                el: this.$('div[name=details]'),
                model: this.model,
                template: $('#makerdetails-template').html()
            });
            this._detailsView.render();

            this._items = new MakerItems([], { maker: this.model });
            this._items.fetch();

            (new CollectionView({
                model: this._items,
                el: this.$('ul[name=items]'),
                view: StaticView.extend({
                    tagName: 'li',
                    template: '<%-item_name%>',
                    events: { click: 'gotoItem' },
                    gotoItem: function() {
                        gApplication.pushPage(new ItemPage({ model: this.model }));
                    }
                }),
                emptyView: StaticView.extend({
                    tagName: 'li',
                    template: 'This maker does not have any items.'
                })
            })).render();
        },
        showCreateDialog: function() {
            var newItem = new Item({ maker_id: this.model.get('maker_id') });
            var m = new Modal({
                buttons: [ StandardButton.cancel(), StandardButton.create() ],
                model: newItem,
                view: ItemForm
            });
            gApplication.modal(m);
            this.listenTo(m, 'finished', this._items.fetch.bind(this._items));
        },
        template: _.template($('#makerpage-template').html()),
        render: function() {
        }
    }
    );

var TypePage = PageView.extend(
    {
        pageTitle: function() { return this.model.get('type_name'); },
        events: {
            'click button[name=new-item]': 'showCreateDialog'
        },
        initialize: function() {
            PageView.prototype.initialize.apply(this, arguments);
            this.model.fetch();

            this.$el.html(this.template(this.templateParams()));

            //this._detailsView = new StaticView({
                //el: this.$('div[name=details]'),
                //model: this.model,
                //template: $('#typedetails-template').html()
            //});
            //this._detailsView.render();

            this._items = new TypeItems([], { type: this.model });
            this._items.fetch();

            (new CollectionView({
                model: this._items,
                el: this.$('ul[name=items]'),
                view: StaticView.extend({
                    tagName: 'li',
                    template: '<%-item_name%>',
                    events: { click: 'gotoItem' },
                    gotoItem: function() {
                        gApplication.pushPage(new ItemPage({ model: this.model }));
                    }
                }),
                emptyView: StaticView.extend({
                    tagName: 'li',
                    template: 'There are no items of this type.'
                })
            })).render();
        },
        showCreateDialog: function() {
            var newItem = new Item({ type_id: this.model.get('type_id') });
            var m = new Modal({
                buttons: [ StandardButton.cancel(), StandardButton.create() ],
                model: newItem,
                view: ItemForm
            });
            gApplication.modal(m);
            this.listenTo(m, 'finished', this._items.fetch.bind(this._items));
        },
        template: _.template($('#typepage-template').html()),
        render: function() {
        }
    }
    );

var ItemForm = StaticView.extend(
    {
        initialize: function(options) {
            StaticView.prototype.initialize.apply(this, arguments);
            StaticView.prototype.render.apply(this);
            this.on('create', this.save.bind(this));
            this.on('save', this.save.bind(this));
            this._makers = new MakerCollection;
            this._makers.fetch({
                success: (function() {
                    this.$('select[name=maker_id]').val(this.model.get('maker_id'));
                }).bind(this)
            });
            this._types = new TypeCollection;
            this._types.fetch({
                success: (function() {
                    this.$('select[name=type_id]').val(this.model.get('type_id'));
                }).bind(this)
            });

            this._messageBox =
                new MessageBox({ el: this.$('div[name=messagebox]') });

            var collections = new CollectionCollection;
            var collectionView = this._collectionView = new CheckedCollectionView({
                el: this.$('ul[name=collections]'),
                model: collections,
                view: StaticView.extend({
                    tagName: 'li',
                    template: '<label><input type="checkbox"></input><%-collection_name%></label>',
                    events: {
                        'change input': function() {
                            console.log('input change')
                            makersView.render();
                            typesView.render();
                        }
                    }
                })
            });
            collections.fetch({
                success: (function() {
                    collectionView.setChecked(
                        new CollectionCollection(this.model.get('collections'))
                    );
                    makersView.render();
                    typesView.render();
                }).bind(this)
            });

            var makersView = new (CollectionView.extend({
                render: function() {
                    var val = this.$el.val();
                    CollectionView.prototype.render.apply(this, arguments);
                    this.$el.val(val);
                }
            }))({
                el: this.$('select[name=maker_id]'),
                model: this._makers,
                filter: (function(maker) {
                    // Maker is unknown or a related collection is checked.
                    return (maker.id == 0) || maker.get('collections').length == 0 ||
                        this.model.get('maker_id') == maker.id ||
                        (_.intersection(
                            collectionView.checkedIds(),
                            (new CollectionCollection(maker.get('collections')))
                                .pluck('collection_id')
                        ).length > 0);
                }).bind(this),
                view: StaticView.extend({
                    tagName: 'option',
                    attributes: function() {
                        return {
                            value: this.model.get('maker_id')
                        };
                    },
                    template: '<%-maker_name%>'
                })
            });
            makersView.render();
            var typesView = new (CollectionView.extend({
                render: function() {
                    var val = this.$el.val();
                    CollectionView.prototype.render.apply(this, arguments);
                    this.$el.val(val);
                }
            }))({
                el: this.$('select[name=type_id]'),
                model: this._types,
                filter: (function(type) {
                    console.log('filter type', type.attributes, collectionView.checkedIds(), (new CollectionCollection(type.get('collections')))
                                .pluck('collection_id'))
                    // Maker is unknown or a related collection is checked.
                    return (type.id == 0) || type.get('collections').length == 0 ||
                        this.model.get('type_id') == type.id ||
                        (_.intersection(
                            collectionView.checkedIds(),
                            (new CollectionCollection(type.get('collections')))
                                .pluck('collection_id')
                        ).length > 0);
                }).bind(this),
                view: StaticView.extend({
                    tagName: 'option',
                    attributes: function() {
                        return {
                            value: this.model.get('type_id')
                        };
                    },
                    template: '<%-type_name%>'
                })
            });
            typesView.render();
        },
        template: _.template($('#itemform-template').html()),
        templateParams: function() {
            return _(_.clone(this.model.attributes))
                .extend({ isNew: this.model.isNew() });
        },
        save: function() {
            this.model.set({
                item_name: this.$('input[name=item_name]').val(),
                maker_id: this.$('select[name=maker_id]').val(),
                type_id: this.$('select[name=type_id]').val(),
                item_notes: this.$('textarea[name=item_notes]').val(),
                item_year: this.$('input[name=item_year]').val(),
                item_cond: this.$('select[name=item_cond]').val(),
                item_cond_notes: this.$('textarea[name=item_cond_notes]').val(),
                collections: this._collectionView.checked()
            });
            this.model.save(
                {},
                {
                    success: (function() {
                        this.trigger('finished');
                    }).bind(this),
                    error: (function() {
                        this._messageBox.displayError('Failed to save item');
                    }).bind(this)
                }
                );
        },
        render: function() {
        }
    }
    );

var ImageUploadForm = StaticView.extend(
    {
        initialize: function(options) {
            StaticView.prototype.initialize.apply(this, arguments);
            this._uploadUrl = options.url;
            this.on('save', this.save.bind(this));
            this.render();
        },
        template: _.template($('#imageupload-template').html()),
        render: function() {
            this.$el.html(this.template(this.templateParams()));
            this._messageBox =
                new MessageBox({ el: this.$('div[name=messagebox]') });
        },
        save: function() {
            var modal = this;
            var reqListener = function() {
                //el.disabled = false;
                if(this.response['error'])
                    modal._messageBox.displayError(
                        'Upload failed: ' + this.response.error
                        );
                else
                    modal.trigger('finished');
                    //messageBox.displaySuccess('Upload complete');
            };
            var xhr = new XMLHttpRequest();
            xhr.open(
                    'post',
                    'item/' + this.model.get('item_id') + '/image',
                    true
                    );
            xhr.onload = reqListener;
            var formData = new FormData(this.el);
            this._messageBox.displayInformation('Uploading...');
            xhr.send(formData);
        }
    }
    );

var ItemImageForm = StaticView.extend(
    {
        initialize: function() {
            StaticView.prototype.initialize.apply(this, arguments);
            this.on('destroy', this.showDestroyDialog.bind(this));
            this.on('save', this.save.bind(this));
        },
        template: $('#itemimageform-template').html(),
        templateParams: function() {
            return _(_.clone(this.model.attributes)).extend({
                url: this.model.urlMedium(),
                alt: this.model.get('attachment_title')
            });
        },
        showDestroyDialog: function() {
            gApplication.modal(
                new ConfirmModal({
                    message: 'Are you sure you want to delete this image?',
                    callback: (function() {
                        this.model.destroy();
                        this.trigger('finished');
                    }).bind(this)
                })
                );
        },
        save: function() {
            this.model.set({
                attachment_title: this.$('input[name=attachment_title]').val()
            });
            this.model.save(
                {},
                {
                    success: (function() {
                        this.trigger('finished');
                    }).bind(this)
                }
                );
        }
    }
    );

var ItemPage = PageView.extend(
    {
        pageTitle: function() { return this.model.get('item_name'); },
        initialize: function() {
            PageView.prototype.initialize.apply(this, arguments);
            this.model.fetch();

            this._imageCollection = new ItemImageCollection([], { item: this.model });
            this._imageCollection.fetch();

            this.render();
        },
        render: function() {
            this.$el.html(this.template(this.templateParams()));
            (new CollectionView({
                model: this._imageCollection,
                el: this.$('ul[name=images]'),
                view: StaticView.extend({
                    tagName: 'li',
                    template: '<img src="<%-url%>" alt="<%-alt%>">',
                    templateParams: function() {
                        return {
                            url: this.model.urlThumb(),
                            alt: this.model.get('attachment_title')
                        };
                    },
                    events: { click: 'showDialog' },
                    showDialog: function() {
                        // Show the dialog for an image connected to an item.
                        //this.model.fetch();
                        var attachment = this.model;
                        var m = new Modal({
                            buttons: [ StandardButton.close(), StandardButton.destroy(), StandardButton.save() ],
                            model: this.model,
                            view: ItemImageForm
                        });
                        gApplication.modal(m);
                    }
                })
            })).render();
        },
        template: _.template($('#itempage-template').html()),
        events: {
            'click button[name=edit]': 'edit',
            'click button[name=addimage]': 'addImage',
            'click button[name=destroy]': 'destroy'
        },
        edit: function() {
            var m = new Modal({
                buttons: [ StandardButton.cancel(), StandardButton.save() ],
                model: this.model,
                view: ItemForm
            });
            gApplication.modal(m);
            this.listenTo(
                m,
                'finished',
                (function() { this.model.fetch(); this.render(); }).bind(this)
                );
        },
        addImage: function() {
            var m = new Modal({
                buttons: [ StandardButton.cancel(), StandardButton.save() ],
                model: this.model,
                view: ImageUploadForm
            });
            gApplication.modal(m);
            this.listenTo(
                m,
                'finished',
                (function() { this._imageCollection.fetch(); }).bind(this)
                );
        },
        destroy: function() {
            gApplication.modal(
                new ConfirmModal({
                    message: 'Are you sure you want to delete "' + this.model.get('item_name') + '"?',
                    callback: (function() {
                        this.model.destroy({
                            success: function() {
                                gApplication.popPage();
                            }
                        });
                    }).bind(this)
                })
                );
        }
    }
    );

var TypeCollectionPage = PageView.extend(
    {
        pageTitle: 'Types',
        initialize: function() {
            PageView.prototype.initialize.apply(this, arguments);
            PageView.prototype.render.apply(this, arguments);
            var model = new TypeCollection;
            model.fetch();
            (new CollectionView({
                el: this.$('ul[name=types]'),
                model: model,
                view: StaticView.extend({
                    tagName: 'li',
                    template: '<%-type_name%>',
                    events: { click: 'gotoType' },
                    gotoType: function() {
                        gApplication.pushPage(
                            new TypePage({ model: this.model })
                            );
                    }
                })
            })).render();
        },
        render: function() {},
        template: $('#typecollectionpage-template').html()
    }
    );

var TimelinePage = PageView.extend(
    {
        pageTitle: 'Timeline',
        initialize: function() {
            PageView.prototype.initialize.apply(this, arguments);
            PageView.prototype.render.apply(this, arguments);
            var items = new ItemCollection(
                [],
                {
                    comparator: function(x) {
                        return - x.get('item_year');
                    }
                }
                );
            items.fetch();
            (new CollectionView({
                el: this.$('ul[name=items]'),
                model: items,
                view: StaticView.extend({
                    tagName: 'li',
                    template: '<%-item_year%>: <%-item_name%> (<%-maker_name%>)',
                    events: { click: 'gotoItem' },
                    gotoItem: function() {
                        gApplication.pushPage(
                            new ItemPage({ model: this.model })
                            );
                    }
                })
            })).render();
        },
        template: $('#timelinepage-template').html(),
        render: function() {}
    }
    );

var MakerCollectionPage = PageView.extend(
    {
        pageTitle: 'Makers',
        initialize: function() {
            PageView.prototype.initialize.apply(this, arguments);
            PageView.prototype.render.apply(this, arguments);
            var model = new MakerCollection;
            model.fetch();
            (new CollectionView({
                el: this.$('ul[name=makers]'),
                model: model,
                view: StaticView.extend({
                    tagName: 'li',
                    template: '<%-maker_name%>',
                    events: { click: 'gotoMaker' },
                    gotoMaker: function() {
                        gApplication.pushPage(
                            new MakerPage({ model: this.model })
                            );
                    }
                })
            })).render();
        },
        render: function() {},
        template: $('#makercollectionpage-template').html()
    }
    );

var HomePage = PageView.extend(
    {
        pageTitle: 'Collection',
        events: {
            'click [name=new-item]': 'showCreateDialog',
            'click [name=makercollection]': 'gotoMakers',
            'click [name=typecollection]': 'gotoTypes',
            'click [name=timeline]': 'gotoTimeline'
        },
        initialize: function(options) {
            PageView.prototype.initialize.apply(this, arguments);

            this._topMakers = new MakerCollection(
                [],
                {
                    // Sorted by popularity (as item_count).
                    comparator: function(maker) {
                        return - maker.get('item_count');
                    }
                }
                );
            this._topMakers.fetch();

            this._topTypes = new TypeCollection(
                [],
                {
                    comparator: function(type) {
                        return - type.get('item_count');
                    }
                }
                );
            this._topTypes.fetch();

            this.$el.html(this.template());
            (new StaticView({
                el: this.$('[name=application_title]'),
                template: '<%-collection_name%>',
                model: gApplication.options()
            })).render();

            // View of the top 5 makers (limited to 5 here, the model is
            // already sorted).
            var makersList = new CollectionView(
                {
                    el: this.$('#top-makers'),
                    model: this._topMakers,
                    limit: 5,
                    view: StaticView.extend(
                        {
                            tagName: 'li',
                            template: '<%-maker_name%>',
                            events: { 'click': 'gotoMaker' },
                            gotoMaker: function() {
                                gApplication.pushPage(
                                    new MakerPage({ model: this.model })
                                    );
                            }
                        }
                        )
                }
                );
            makersList.render();
            var typesList = new CollectionView(
                {
                    el: this.$('#top-types'),
                    model: this._topTypes,
                    limit: 5,
                    view: StaticView.extend({
                        tagName: 'li',
                        template: '<%-type_name%>',
                        events: { 'click': 'gotoType' },
                        gotoType: function() {
                            gApplication.pushPage(
                                new TypePage({ model: this.model })
                                );
                        }
                    })
                }
                );
            typesList.render();
        },
        render: function() {
        },
        showCreateDialog: function() {
            var m = new Modal({
                buttons: [ StandardButton.cancel(), StandardButton.create() ],
                model: new Item,
                view: ItemForm
            });
            gApplication.modal(m);
            this.listenTo(
                    m,
                    'finished',
                    (function() {
                        this._topMakers.fetch();
                        this._topTypes.fetch();
                    }).bind(this)
                    );
        },
        gotoMakers: function() {
            gApplication.pushPage(MakerCollectionPage);
        },
        gotoTypes: function() {
            gApplication.pushPage(TypeCollectionPage);
        },
        gotoTimeline: function() {
            gApplication.pushPage(TimelinePage);
        },
        template: _.template($('#homepage-template').html())
    }
    );
