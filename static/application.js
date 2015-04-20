/*
 * Functions for accessing the browser's local storage API.  Some old browsers
 * do not support local storage; fall back to a global array if this is the
 * case.
 */
var storage = {};

if(
    (function() {
        console.log('testing localStorage');
        try {
            localStorage.setItem('mod', 'mod');
            localStorage.removeItem('mod');
            console.log('found localStorage');
            return true;
        } catch(exception) {
            console.log('did not find localStorage');
            return false;
        }
    }()
    ))
{
    storage = {
        get: function(key) {
            return window.localStorage.getItem(key);
        },
        has: function(key) {
            return _.has(gStorage, key);
        },
        set: function(key, value) {
            window.localStorage.setItem(key, value);
        },
        remove: function(key) {
            window.localStorage.setItem(key, null);
        }
    };
} else {
    var gStorage = {};
    storage = {
        get: function(key) {
            return gStorage[key];
        },
        has: function(key) {
            return (window.localStorage.getItem(key) != null);
        },
        set: function(key, value) {
            gStorage[key] = value;
        },
        remove: function(key) {
            delete gStorage['key'];
        }
    };
}

/*
 * Set the 'Authorization' header on an outgoing XMLHttpRequest.
 */
var setHeader = function(xhr) {
    xhr.setRequestHeader('Authorization', storage.get('token'));
};

{
    var oldSync = Backbone.sync;
    Backbone.sync = function(method, model, options) {
        if(_.isUndefined(options))
            options = {};
        options.beforeSend = setHeader;
        oldSync(method, model, options);
    };
}

/*
 * Push a sign in page to the application whenever a jQuery request receives a
 * 403 error.
 */
//$(document).ajaxError(
        //function (e, xhr, options) {
            //gApplication.pushPage(SignInPage);
        //}
        //);

/*
 * Make a JSONRPC request.
 *
 * 'options' is a map of:
 *
 * success: Function called with the JSONRPC result if the request is successful.
 * error: Function called with the JSONRPC error message if the request fails.
 * url: JSONRPC endpoint.
 * method: JSONRPC method to call.
 * params: Parameters to the method.
 */
var jsonRpc = function(options) {
    var url = _.has(options, 'url')?options.url:'/api_call';
    var req = _.has(options, 'xhr')?options.xhr:new XMLHttpRequest;
    var reqListener = function() {
        switch(this.status) {
            case 200:
                var jsonIn = JSON.parse(this.responseText);
                if(_.has(jsonIn, 'result'))
                    options.success(jsonIn.result);
                else if(_.has(jsonIn, 'error'))
                    options.error(jsonIn.error);
                else
                    options.error();
                break;
            case 403:
                gApplication.pushPage(SignInPage);
                break;
        }
    }

    var requestContent = _.pick(options, 'method', 'params');
    console.log(requestContent);
    req.open('post', url, true);
    setHeader(req);
    req.onload = reqListener;
    req.send(JSON.stringify(requestContent));
};

/*
 * Single message to be visualised in a MessageBox.
 */
var Message = Backbone.Model.extend(
    {
        defaults: { severity: 'information', message: '', closeButton: true },
        timeout: function(delay) {
            setTimeout(
                (function() {
                    this.trigger('fadeout');
                    setTimeout(this.destroy.bind(this), 1000);
                }).bind(this),
            delay
            );
        }
    }
    );

var MessageCollection = Backbone.Collection.extend(
    {
        model: Message
    }
    );

/*
 * A single message displayed in a box with appropriate styling.
 */
var MessageView = StaticView.extend(
    {
        initialize: function() {
            StaticView.prototype.initialize.apply(this, arguments);
            this.render();
        },
        model: Message,
        className: function() {
            return 'messagebox messagebox-' + this.model.get('severity');
        },
        fadeout: function() {
            this.$el.attr('style', 'opacity: 0;')
        },
        events: {
            'click button[name=close]': function() { this.model.destroy(); }
        },
        template: '<span><%-message%></span><button name="close">Close</button>'
    }
    );

var MessageCollectionView = CollectionView.extend(
    {
        view: MessageView,
        initializeView: function(view) {
            this.listenTo(view.model, 'fadeout', view.fadeout.bind(view));
        }
    }
    );

/*
 * Default message box dismissal timeout in milliseconds.
 */
var defaultTimeout = 5000;

/*
 * View onto a list of messages which inform the user of recent events.
 * Messages may be dismissed by clicking the (optional) close button, or will
 * be dismissed automatically after a timeout.
 */
var MessageBox = StaticView.extend(
    {
        initialize: function(options) {
            StaticView.prototype.initialize.apply(this, arguments);
            if(!_.has(this, 'model'))
                this.model = new MessageCollection;
            this._collectionView = new MessageCollectionView({ model: this.model });
            this._collectionView.render();
            this.render();
        },
        displayError: function(str) {
            var message = new Message({ severity: 'error', message: str });
            message.timeout(defaultTimeout);
            this.model.add(message);
        },
        displayInformation: function(str) {
            var message = new Message({ severity: 'information', message: str });
            message.timeout(defaultTimeout);
            this.model.add(message);
        },
        displaySuccess: function(str) {
            var message = new Message({ severity: 'success', message: str });
            message.timeout(defaultTimeout);
            this.model.add(message);
        },
        displayWarning: function(str) {
            var message = new Message({ severity: 'warning', message: str });
            message.timeout(defaultTimeout);
            this.model.add(message);
        },
        render: function() {
            this.$el.empty();
            this.$el.append(this._collectionView.$el);
        }
    }
    );


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

