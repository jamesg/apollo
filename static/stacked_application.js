var StaticView = Backbone.View.extend(
    {
        initialize: function(options) {
            Backbone.View.prototype.initialize.apply(this, arguments);
            if(_.has(this, 'model') && !_.isUndefined(this.model))
                this.listenTo(this.model, 'change', this.render.bind(this));
            if(!_.isUndefined(options) && _(options).has('template'))
                this.template = options.template;
        },
        template: '',
        templateParams: function() {
            return (_(this).has('model') && !_.isUndefined(this.model))?
                this.model.toJSON():{};
        },
        render: function() {
            if(_.isFunction(this['template']))
                this.$el.html(this.template(this.templateParams()));
            else
                this.$el.html(_.template(this.template)(this.templateParams()));
        }
    }
    );

var Modal = Backbone.View.extend(
    {
        /*
         * options - map of:
         *     buttons - buttons to display
         *     view - view constructor
         *     model - passed to the view
         */
        initialize: function(options) {
            Backbone.View.prototype.initialize.apply(this, arguments);

            this._buttons = coalesce(options['buttons'], this['buttons'], { close: true });

            this.$el.append(_.template(this.template)(this.templateParams()));

            var cons = coalesce(options['view'], this['view']);
            this._view = new cons({ el: this.contentEl(), model: this['model']});
            this._view.render();
            this.listenTo(this._view, 'finished', this.finish.bind(this));
        },
        className: 'modal',
        template: $('#modal-template').html(),
        templateParams: function() {
            return { buttons: this._buttons };
        },
        contentEl: function() {
            return this.$('*[name=modal-content]')[0];
        },
        events: {
            'click button[name=cancel]': 'remove',
            'click button[name=close]': 'remove',
            'click button[name=delete]': function() { return this._end('destroy'); },
            'click button[name=no]': function() { return this._end('no') },
            'click button[name=create]': function() { return this._end('create'); },
            'click button[name=save]': function() { return this._end('save'); },
            'click button[name=yes]': function() { return this._end('yes'); },
            'submit form': function() { return this._end('save'); }
        },
        finish: function() {
            this.remove();
            this.trigger('finished');
        },
        _end: function(act) {
            if(_(this._buttons[act]).isFunction())
                this._buttons[act]();
            this._view.trigger(act);
            return false;
        }
    }
    );

var ConfirmModal = Modal.extend(
    {
        initialize: function(options) {
            Modal.prototype.initialize.apply(this, arguments);
            this._callback =
                coalesce(options['callback'], this['callback'], function() {});
            this._view.message = options['message'];
            this._view.render();
        },
        events: {
            'click button[name=no]': 'no',
            'click button[name=yes]': 'yes'
        },
        yes: function() {
            this._callback();
            this.remove();
        },
        no: function() {
            this.remove();
        },
        buttons: { yes: true, no: true },
        view: StaticView.extend({
            template: '<p><%-message%>',
            templateParams: function() { return { message: this.message }; },
            message: 'Are you sure?'
        })
    }
    );

/*
 * Return the first defined, non-null argument.  Call with any number of
 * arguments.
 *
 * If no arguments are defined and non-null, return null.
 */
var coalesce = function() {
    for(a in arguments)
        if(!_.isNull(arguments[a]) && !_.isUndefined(arguments[a]))
            return arguments[a];
    return null;
};

var CollectionView = Backbone.View.extend(
    {
        initialize: function(options) {
            this.view = coalesce(
                options['view'],
                this['view'],
                function() { console.log('warning: view not defined'); }
                );
            this.emptyView = coalesce(options['emptyView'], this['emptyView']);
            this._offset = coalesce(options['offset'], 0);
            this._limit = coalesce(options['limit'], -1);
            this.listenTo(this.model, 'add', this.render);
            this.listenTo(this.model, 'remove', this.render);
            this.listenTo(this.model, 'reset', this.render);
        },
        constructView: function(model) {
            var view = new this.view({ model: model });
            this.initializeView(view);
            return view;
        },
        initializeView: function(view) {
        },
        emptyView: StaticView.extend({}),
        render: function() {
            // Replace the content of the element with a newly constructed view
            // for each model in the collection.
            //
            // TODO reuse old models, both for performance and to enable
            // editable views.
            this.$el.empty();
            var models;

            if(this._limit >= 0)
                models = this.model.slice(this._offset, this._offset + this._limit);
            else
                models = this.model.slice(this._offset);

            if(models.length == 0) {
                var dv = new this.emptyView;
                dv.render();
                this.$el.append(dv.el);
            } else {
                _(models).each(
                    function(model) {
                        var dv = this.constructView(model);
                        dv.render();
                        this.$el.append(dv.el);
                    },
                    this
                    );
            }
        }
    }
    );

var Breadcrumb = Backbone.Model.extend(
    {
        defaults: {
            page_title: '',
            view: null
        }
    }
    );

var BreadcrumbCollection = Backbone.Collection.extend(
    {
        model: Breadcrumb
    }
    );

var BreadcrumbView = StaticView.extend(
    {
        tagName: 'li',
        revisit: function revisit() {
            gApplication.revisit(this.model);
        },
        events: {
            'click a': 'revisit'
        },
        template:
            '<span aria-hidden="aria-hidden" data-glyph="chevron-right" class="oi"> </span>' +
            '<a><%-page_title%></a>  '
    }
    );

var BreadcrumbsView = CollectionView.extend(
    {
        view: BreadcrumbView,
        initialize: function(options) {
            this.application = options.application;
            CollectionView.prototype.initialize.apply(this, arguments);
            this.render();
        },
        initializeView: function(view) {
            view.application = this.application;
            view.render();
        },
        tagName: 'ul'
    }
    );

var StackedApplication = function(homeView) {
    this._homeView = homeView;

    this.breadcrumbs = new BreadcrumbCollection;
    this.navigation = new CollectionView(
            {
                el: $('#navigation'),
                model: this.breadcrumbs,
                view: BreadcrumbView
            }
            );

    //this.goHome();
};

//_.extend(StackedApplication, Backbone.Events);

StackedApplication.prototype._setElement = function(el) {
    document.getElementById('template-content').innerHTML = '';
    document.getElementById('template-content').appendChild(el);
};

StackedApplication.prototype._createPage = function(constructor) {
    return (_.isFunction(constructor))?
        (new constructor({ application: this })):
        constructor;
};

StackedApplication.prototype._createBreadcrumb = function(view) {
    return new Breadcrumb(
        {
            page_title: _.isFunction(view.pageTitle)?
                    view.pageTitle():view.pageTitle,
            view: view
        }
        );
};

StackedApplication.prototype._setPage = function(view) {
    this._setElement(view.el);
};

StackedApplication.prototype.goHome = function() {
    this.breadcrumbs.reset();
    var home = this._createPage(this._homeView);
    this.breadcrumbs.add(this._createBreadcrumb(home));
    this._setPage(home);
};

StackedApplication.prototype.gotoPage = function(constructor) {
    this.breadcrumbs.reset();
    var home = this._createPage(HomePage);
    this.breadcrumbs.add(this._createBreadcrumb(home));
    this.pushPage(constructor);
};

StackedApplication.prototype.pushPage = function(constructor) {
    var view = this._createPage(constructor);
    this.breadcrumbs.add(this._createBreadcrumb(view));
    this._setPage(view);
};

StackedApplication.prototype.popPage = function() {
    this.breadcrumbs.pop();
    if(this.breadcrumbs.length == 0)
        this.goHome();
    else
        this._setPage(this.breadcrumbs.at(this.breadcrumbs.length-1).get('view'));
    this.currentPage().reset();
};

StackedApplication.prototype.currentPage = function() {
    if(this.breadcrumbs.length < 1)
        throw 'there is not current page';
    return this.breadcrumbs.at(this.breadcrumbs.length-1).get('view');
};

StackedApplication.prototype.revisit = function(breadcrumb) {
    var i = 0;
    while(i < this.breadcrumbs.length) {
        if(this.breadcrumbs.at(i) == breadcrumb) {
            this.breadcrumbs.remove(
                    this.breadcrumbs.slice(i+1, this.breadcrumbs.length)
                    );
            this._setPage(this.breadcrumbs.at(i).get('view'));
            this.currentPage().reset();
            return;
        }
        ++i;
    }
};

/*
 * Display a modal dialog.
 */
StackedApplication.prototype.modal = function(modal) {
    $('#modal-container').append(modal.el);
};

/*!
 * \brief Handle an authentication error by displaying a sign in page over the
 * current page.  Will only display a sign in page if none is currently
 * displayed.
 */
StackedApplication.prototype.authenticationError = function() {
    if(!(this.currentPage() instanceof AuthenticationRequiredPage)) {
        this.pushPage(AuthenticationRequiredPage);
    }
};

var PageView = StaticView.extend(
    {
        pageTitle: 'Untitled Page',
        reset: function() { }
    }
    );

