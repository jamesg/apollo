var MakerForm = StaticView.extend(
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
                { maker_name: this.$('input[name=maker_name]').val() }
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
        template: _.template($('#maker-form-template').html())
    }
    );

var TypeForm = StaticView.extend(
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
            this.$el.html(
                _.template(this.template).apply(this, [this.templateParams()])
                );
            this._messageBox = new MessageBox({ el: this.$('div[name=messagebox]') });
        },
        save: function() {
            this.model.set(
                { type_name: this.$('input[name=type_name]').val() }
                );
            if(this.model.isValid())
                this.model.save(
                    {},
                    {
                        success: (function() {
                            console.log('finish type save');
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

            this._makers = new MakerCollection();
            this._makers.comparator = function(x) {
                return (x.get('maker_id')==0)?0:x.get('maker_name');
            };
            this._makers.fetch();
            this._types = new TypeCollection;
            this._types.comparator = function(x) {
                return (x.get('type_id')==0)?0:x.get('type_name');
            };
            this._types.fetch();

            this.$el.html(this.template());
            (new StaticView({
                el: this.$('[name=application_title]'),
                template: '<%-collection_name%>',
                model: gApplication.options()
            })).render();


            this.$('button[name=new-maker]').on(
                'click',
                (function() {
                    var m = new Modal({
                        buttons: [ StandardButton.cancel(), StandardButton.create() ],
                        model: new Maker,
                        view: MakerForm
                    });
                    gApplication.modal(m);
                    this.listenTo(m, 'finished', this._makers.fetch.bind(this._makers));
                }).bind(this)
                );
            this.$('button[name=new-type]').on(
                'click',
                (function() {
                    var m = new Modal(
                        {
                            buttons: [ StandardButton.cancel(), StandardButton.create() ],
                            model: new Type,
                            view: TypeForm
                        }
                        );
                    gApplication.modal(m);
                    this.listenTo(m, 'finished', this._types.fetch.bind(this._types));
                }).bind(this)
                );

            var page = this;
            (new CollectionView({
                el: this.$('ul[name=makers]'),
                model: this._makers,
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
                })).render();
            (new CollectionView({
                el: this.$('ul[name=types]'),
                model: this._types,
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
                })).render();
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

