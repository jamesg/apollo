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
                    message: 'Are you sure you want to delete \'' + this.model.get('maker_name') +'\'?',
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
                    message: 'Are you sure you want to delete \'' + this.model.get('type_name') +'\'?',
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
        initialize: function(options) {
            PageView.prototype.initialize.apply(this, arguments);
            this._application = options.application;

            this._makers = new MakerCollection();
            this._makers.fetch();
            this._types = new TypeCollection();
            this._types.fetch();

            this.$el.html(this.template());

            this.$('button[name=new-maker]').on(
                'click',
                (function() {
                    var m = new Modal({
                        buttons: { cancel: true, create: true },
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
                            buttons: { cancel: true, create: true },
                            model: new Type,
                            view: TypeForm
                        }
                        );
                    this._application.modal(m);
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
                                buttons: { cancel: true, destroy: true, save: true },
                                model: this.model,
                                view: MakerForm
                        });
                        page._application.modal(m);
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
                                buttons: { cancel: true, destroy: true, save: true },
                                model: this.model,
                                view: TypeForm
                            }
                            );
                        page._application.modal(m);
                        page.listenTo(m, 'finished', page._types.fetch.bind(page._types));
                    }
                    })
                })).render();
        }
    }
    );

