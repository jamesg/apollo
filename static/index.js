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
                buttons: { cancel: true, create: true },
                model: newItem,
                view: ItemForm
            });
            gApplication.modal(m);
            this.listenTo(m, 'finished', this._items.fetch.bind(this._items));
        },
        template: _.template($('#makerpage-template').html())
    }
    );

var ItemForm = StaticView.extend(
    {
        initialize: function(options) {
            StaticView.prototype.initialize.apply(this, arguments);
            this.on('create', this.save.bind(this));
            this.on('save', this.save.bind(this));
            this._makers = new MakerCollection;
            this._makers.fetch();
            this._types = new TypeCollection;
            this._types.fetch();
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
                item_cond_notes: this.$('textarea[name=item_cond_notes]').val()
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
            this.$el.html(this.template(this.templateParams()));
            this._messageBox =
                new MessageBox({ el: this.$('div[name=messagebox]') });
            new CollectionView({
                el: this.$('select[name=maker_id]'),
                model: this._makers,
                view: StaticView.extend({
                    tagName: 'option',
                    attributes: function() {
                        return { value: this.model.get('maker_id') };
                    },
                    template: '<%-maker_name%>'
                })
            }).render();
            new CollectionView({
                el: this.$('select[name=type_id]'),
                model: this._types,
                view: StaticView.extend({
                    tagName: 'option',
                    attributes: function() {
                        return { value: this.model.get('type_id') };
                    },
                    template: '<%-type_name%>'
                })
            }).render();
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
                    '/item/' + this.model.get('item_id') + '/image',
                    true
                    );
            xhr.onload = reqListener;
            var formData = new FormData(this.el);
            this._messageBox.displayInformation('Uploading...');
            xhr.send(formData);

            return false;
        }
    }
    );

var ItemPage = PageView.extend(
    {
        pageTitle: function() { return this.model.get('item_name'); },
        initialize: function() {
            PageView.prototype.initialize.apply(this, arguments);
            this.model.fetch();

            this.$el.html(this.template(this.templateParams()));

            var imageCollection = new ItemImageCollection({ item: this.model });
            imageCollection.fetch();
            var imageCollectionView = new CollectionView({
                model: imageCollection,
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
                        var m = new Modal({
                            model: this.model,
                            view: StaticView.extend({
                                template: $('#itemimagemodal-template').html(),
                                templateParams: function() {
                                    return {
                                        url: this.model.urlMedium(),
                                        alt: this.model.get('attachment_title')
                                    };
                                }
                            })
                        });
                        gApplication.modal(m);
                    }
                })
            })
        },
        template: _.template($('#itempage-template').html()),
        events: {
            'click button[name=edit]': 'edit',
            'click button[name=addimage]': 'addImage'
        },
        edit: function() {
            var m = new Modal({
                buttons: { cancel: true, save: true },
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
                buttons: { cancel: true, save: true },
                model: this.model,
                view: ImageUploadForm
            });
            gApplication.modal(m);
            this.listenTo(
                m,
                'finished',
                (function() { this.model.fetch(); this.render(); }).bind(this)
                );
        }
    }
    );

var HomePage = PageView.extend(
    {
        pageTitle: 'Computer Collection',
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
                    view: StaticView.extend({ tagName: 'li', template: '<%-type_name%>' })
                }
                );
            typesList.render();
        },
        template: _.template($('#homepage-template').html())
    }
    );

