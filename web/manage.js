var CollectionForm = StaticView.extend(
    {
        initialize: function(options) {
            StaticView.prototype.initialize.apply(this, arguments);
            this.model.fetch();
            this.render();
            this.on('create', this.save.bind(this));
            this.on('save', this.save.bind(this));
            this.on('destroy', this.destroy.bind(this));
        },
        render: function() {
            this.$el.html(this.template(this.templateParams()));
            this._messageBox =
                new MessageBox({ el: this.$('div[name=messagebox]') });
        },
        save: function() {
            this.model.set(
                { collection_name: this.$('input[name=collection_name]').val() }
                );
            if(this.model.isValid())
                this.model.save(
                    {},
                    {
                        success: (function() {
                            this.trigger('finished');
                        }).bind(this),
                        error: (function() {
                            this._messageBox.displayError('Failed to save maker');
                        }).bind(this)
                    }
                    );
            else
                this._messageBox.displayError('Validation error');
        },
        destroy: function() {
            gApplication.modal(
                new ConfirmModal({
                    message: 'Are you sure you want to delete \'' +
                        this.model.get('collection_name') +
                        '\'?',
                    callback: (function() {
                        this.model.destroy();
                        this.trigger('finished');
                    }).bind(this)
                }));
        },
        templateParams: function() {
            return _(_.clone(this.model.attributes)).extend(
                    {
                        'isNew': this.model.isNew()
                    }
                    );
        },
        template: _.template($('#collection-form-template').html())
    }
    );

var MakerForm = StaticView.extend(
    {
        initialize: function(options) {
            StaticView.prototype.initialize.apply(this, arguments);
            StaticView.prototype.render.apply(this);
            this._messageBox =
                new MessageBox({ el: this.$('div[name=messagebox]') });
            var collections = new CollectionCollection;
            this._collectionView = new CheckedCollectionView({
                el: this.$('ul[name=collections]'),
                model: collections,
                view: StaticView.extend({
                    tagName: 'li',
                    template: '<label><input type="checkbox"></input><%-collection_name%></label>'
                })
            });
            collections.fetch({
                success: (function() {
                    this._collectionView.setChecked(
                        new CollectionCollection(this.model.get('collections'))
                    );
                }).bind(this)
            });
            this._collectionView.render();
            this.on('create', this.save.bind(this));
            this.on('save', this.save.bind(this));
            this.on('destroy', this.destroy.bind(this));
        },
        save: function() {
            this.model.set({
                maker_name: this.$('input[name=maker_name]').val(),
                collections: this._collectionView.checked()
            });
            if(this.model.isValid())
                this.model.save(
                    {},
                    {
                        success: (function() {
                            this.trigger('finished');
                        }).bind(this),
                        error: (function() {
                            this._messageBox.displayError('Failed to save maker');
                        }).bind(this)
                    }
                    );
            else
                this._messageBox.displayError('Validation error');
        },
        destroy: function() {
            gApplication.modal(
                new ConfirmModal({
                    message: 'Are you sure you want to delete \'' +
                        this.model.get('maker_name') +
                        '\'?  Items made by \'' +
                        this.model.get('maker_name') +
                        '\' will be reclassified as \'Unknown\'',
                    callback: (function() {
                        this.model.destroy();
                        this.trigger('finished');
                    }).bind(this)
                }));
        },
        templateParams: function() {
            return _(_.clone(this.model.attributes)).extend(
                    {
                        'isNew': this.model.isNew()
                    }
                    );
        },
        template: _.template($('#maker-form-template').html()),
        render: function() {
        }
    }
    );

var TypeForm = StaticView.extend(
    {
        initialize: function(options) {
            StaticView.prototype.initialize.apply(this, arguments);
            StaticView.prototype.render.apply(this);
            this._messageBox =
                new MessageBox({ el: this.$('div[name=messagebox]') });
            var collections = new CollectionCollection;
            this._collectionView = new CheckedCollectionView({
                el: this.$('ul[name=collections]'),
                model: collections,
                view: StaticView.extend({
                    tagName: 'li',
                    template: '<label><input type="checkbox"></input><%-collection_name%></label>'
                })
            });
            collections.fetch({
                success: (function() {
                    this._collectionView.setChecked(
                        new CollectionCollection(this.model.get('collections'))
                    );
                }).bind(this)
            });
            this._collectionView.render();
            this.on('create', this.save.bind(this));
            this.on('save', this.save.bind(this));
            this.on('destroy', this.destroy.bind(this));
        },
        render: function() {
        },
        save: function() {
            this.model.set({
                type_name: this.$('input[name=type_name]').val(),
                collections: this._collectionView.checked()
            });
            if(this.model.isValid())
                this.model.save(
                    {},
                    {
                        success: (function() {
                            this.trigger('finished');
                        }).bind(this),
                        error: (function(err) {
                            this._messageBox.displayError('Error saving type');
                            console.log('failure', arguments);
                        }).bind(this)
                    }
                );
            else
                this._messageBox.displayError('Validation error');
        },
        destroy: function() {
            gApplication.modal(
                new ConfirmModal({
                    message: 'Are you sure you want to delete \'' +
                        this.model.get('type_name') +
                        '\'?  Items classified as by \'' +
                        this.model.get('type_name') +
                        '\' will be reclassified as \'Unknown\'',
                    callback: (function() {
                        this.model.destroy();
                        this.trigger('finished');
                    }).bind(this)
                }));
        },
        templateParams: function() {
            return _(_.clone(this.model.attributes)).extend(
                    {
                        'isNew': this.model.isNew()
                    }
                    );
        },
        template: $('#type-form-template').html()
    }
    );

var ManageHomePage = PageView.extend(
    {
        pageTitle: 'Manage Collection',
        render: function() {},
        template: _.template($('#manage-homepage').html()),
        events: {
            'click button[name=show-options]': 'showOptions'
        },
        showOptions: function() {
            gApplication.modal(
                new Modal({
                    buttons: [ StandardButton.save(), StandardButton.cancel() ],
                    view: OptionsForm,
                    model: gApplication.options()
                })
                );
        },
        initialize: function(options) {
            PageView.prototype.initialize.apply(this, arguments);
            PageView.prototype.render.apply(this);

            this._collections = new CollectionCollection;
            this._collections.comparator = function(x) {
                return x.get('collection_name');
            };
            this._collections.fetch();
            this._makers = new MakerCollection;
            this._makers.comparator = function(x) {
                return (x.id == 0) ? 0 : x.get('maker_name');
            };
            this._makers.fetch();
            this._types = new TypeCollection;
            this._types.comparator = function(x) {
                return (x.id == 0) ? 0 : x.get('type_name');
            };
            this._types.fetch();

            // this.$el.html(this.template());
            (new StaticView({
                el: this.$('[name=application_title]'),
                template: '<%-collection_name%>',
                model: gApplication.options()
            })).render();

            var page = this;
            var collectionView = new CheckedCollectionView({
                el: this.$('ul[name=collections]'),
                model: this._collections,
                view: StaticView.extend({
                    tagName: 'li',
                    template: '<label><input type="checkbox" checked></input><%-collection_name%></label>',
                    events: {
                        'change input': function() {
                            makersView.render();
                        }
                    }
                })
            });
            collectionView.render();
            var makersView = new CollectionView({
                el: this.$('ul[name=makers]'),
                model: this._makers,
                filter: (function(maker) {
                    // Maker is unknown or a related collection is checked.
                    return (maker.id == 0) || maker.get('collections').length == 0 ||
                        (_.intersection(
                            collectionView.checkedIds(),
                            (new CollectionCollection(maker.get('collections')))
                                .pluck('collection_id')
                        ).length > 0);
                }).bind(this),
                view: StaticView.extend({
                    tagName: 'li',
                    template: '<%-maker_name%>',
                    events: { 'click': 'gotoMaker' },
                    gotoMaker: function() {
                        var m = new Modal({
                                buttons: [ StandardButton.cancel(), StandardButton.destroy(), StandardButton.save() ],
                                model: this.model,
                                view: MakerForm
                        });
                        gApplication.modal(m);
                        page.listenTo(m, 'finished', page._makers.fetch.bind(page._makers));
                    }
                    })
            });
            makersView.render();
            var typesView = new CollectionView({
                el: this.$('ul[name=types]'),
                model: this._types,
                filter: (function(type) {
                    // Maker is unknown or a related collection is checked.
                    return (type.id == 0) || type.get('collections').length == 0 ||
                        (_.intersection(
                            collectionView.checkedIds(),
                            (new CollectionCollection(type.get('collections')))
                                .pluck('collection_id')
                        ).length > 0);
                }).bind(this),
                view: StaticView.extend({
                    tagName: 'li',
                    template: '<%-type_name%>',
                    events: { 'click': 'gotoType' },
                    gotoType: function() {
                        var m = new Modal(
                            {
                                buttons: [ StandardButton.cancel(), StandardButton.destroy(), StandardButton.save() ],
                                model: this.model,
                                view: TypeForm
                            }
                            );
                        gApplication.modal(m);
                        page.listenTo(m, 'finished', page._types.fetch.bind(page._types));
                    }
                    })
            });
            typesView.render();
        },
        reset: function() {
            this._collections.fetch();
            this._makers.fetch();
            this._types.fetch();
        },
        events: {
            'click button[name=new-collection]': 'newCollection',
            'click button[name=new-maker]': 'newMaker',
            'click button[name=new-type]': 'newType'
        },
        newCollection: function() {
            var m = new Modal({
                buttons: [ StandardButton.cancel(), StandardButton.create() ],
                model: new Collection,
                view: CollectionForm
            });
            gApplication.modal(m);
            this.listenTo(m, 'finished', this._collections.fetch.bind(this._collections));
        },
        newMaker: function() {
            var m = new Modal({
                buttons: [ StandardButton.cancel(), StandardButton.create() ],
                model: new Maker,
                view: MakerForm
            });
            gApplication.modal(m);
            this.listenTo(m, 'finished', this._makers.fetch.bind(this._makers));
        },
        newType: function() {
            var m = new Modal(
                {
                    buttons: [ StandardButton.cancel(), StandardButton.create() ],
                    model: new Type,
                    view: TypeForm
                }
                );
            gApplication.modal(m);
            this.listenTo(m, 'finished', this._types.fetch.bind(this._types));
        },
        render: function() {
        }
    }
);

var OptionsForm = StaticView.extend(
    {
        template: $('#optionsform-template').html(),
        initialize: function(options) {
            StaticView.prototype.initialize.apply(this, arguments);
            this.model.fetch();
            this.render();
            this.on('save', this.save.bind(this));
        },
        save: function() {
            this.model.set(
                {
                    collection_name: this.$('input[name=collection_name]').val()
                }
                );
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
