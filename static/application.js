var TbodyView = CollectionView.extend(
    {
        tagName: 'tbody',
        initialize: function(options) {
            CollectionView.prototype.initialize.apply(this, arguments);
            this.initializeTrView = options.initializeTrView;
        },
        initializeView: function(view) {
            this.listenTo(view, 'click', this.trigger.bind(this, 'click'));
            this.initializeTrView(view);
        }
    }
    );

/*
 * A generic table view that can be adapted to any kind of table with a header.
 * Provide a view constructor for the table header (thead) as the theadView
 * option and a view constructor for the table rows (tr) as the trView option.
 *
 * model: An instance of Backbone.Collection.
 * emptyView: View to use if the table is empty (defaults to displaying no
 * rows).
 * theadView: Header view constructor.
 * trView: Row view constructor.
 */
var TableView = StaticView.extend(
    {
        tagName: 'table',
        initialize: function(options) {
            StaticView.prototype.initialize.apply(this, arguments);

            this._thead = new options.theadView;
            this._thead.render();
            this._tbody = new TbodyView(
                {
                    initializeTrView: this.initializeTrView,
                    view: options.trView,
                    model: this.model
                }
                );
            this._tbody.render();

            this._empty = new options.emptyView;

            this.listenTo(this.model, 'all', this.render);
            this.listenTo(this._tbody, 'click', this.trigger.bind(this, 'click'));

            this.render();
        },
        render: function() {
            this.$el.empty();
            this.$el.append(this._thead.$el);
            if(this.model.length == 0)
                this.$el.append(this._empty.$el);
            else
                this.$el.append(this._tbody.$el);
        },
        initializeTrView: function(trView) {
        }
    }
    );


//var SignInPage = PageView.extend(
    //{
        //pageTitle: 'Sign In',
        //initialize: function(options) {
            //PageView.prototype.initialize.apply(this, arguments);
            //this._application = options.application;
            //this.initRender();
            //this._messageBox = new MessageBox(
                    //{
                        //el: this.$('div[name=messagebox]')
                    //}
                    //);
            //this._messageBox.render();
        //},
        //events: {
            //'submit form': 'signIn'
        //},
        //signIn: function() {
            //var application = this._application;
            //var messageBox = this._messageBox;
            //jsonRpc(
                //{
                    //url: '/auth',
                    //method: 'sign_in',
                    //params: [this.$username.val(), this.$password.val()],
                    //success: function(user) {
                        //storage.set('token', user.token);
                        //application.popPage();
                    //},
                    //error: function(err) {
                        //messageBox.displayError('Sign in failed; ' + err);
                    //}
                //}
                //);
            //return false;
        //},
        //template: _.template($('#signin-page').html()),
        //initRender: function() {
            //this.$el.html(this.template());
            //this.$username = this.$('input[name=username]');
            //this.$password = this.$('input[name=password]');
        //}
    //}
    //);

var CheckedCollectionView = CollectionView.extend(
        {
            // Get a list of models which are checked.
            checked: function() {
                var out = [];
                this.each(
                    function(view) {
                        if(view.$('input[type=checkbox]').prop('checked'))
                            out.push(view.model);
                    }
                );
                return out;
            },
            // Get a list of model ids which are checked.
            checkedIds: function() {
                return _.pluck(this.checked(), 'id');
            },
            // Set the list of checked models using a Backbone collection.
            setChecked: function(collection) {
                this.each(
                    function(view) {
                        view.$('input[type=checkbox]').prop(
                            'checked',
                            collection.some(
                                function(model) { return model.id == view.model.id }
                            )
                        );
                    }
                );
            },
            // Set the list of checked models using an array of ids.
            setCheckedIds: function(ids) {
                this.each(
                    function(view) {
                        this.view.$('input[type=checkbox]').prop(
                            'checked',
                            _.some(ids, function(id) { return id == view.model.id })
                        );
                    }
                );
            }
        }
    );

var Application = function(homeView) {
    StackedApplication.apply(this, arguments);
    this._options = new Options;
    this._options.fetch();
};

Application.prototype = Object.create(StackedApplication.prototype);

/*
 * Get the global application options, such as the collection name.
 */
Application.prototype.options = function() {
    return this._options;
};
