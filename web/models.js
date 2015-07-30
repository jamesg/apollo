/*
 * Get the URI for a REST API handler.
 */
var restUri = function(fragment) {
    return 'api' + fragment;
};

var Collection = RestModel.extend(
    {
        defaults: {
            collection_name: ''
        },
        idAttribute: 'collection_id',
        url: function() {
            return restUri(this.isNew()?'/collection':('/collection/' + this.get('collection_id')));
        }
    }
);

var CollectionCollection = RestCollection.extend(
    {
        model: Collection,
        url: restUri('/collection')
    }
);

var Type = RestModel.extend(
    {
        defaults: {
            type_name: ''
        },
        idAttribute: 'type_id',
        validate: function() {
            var errors = {};
            if(this.get('name') == '')
                errors['name'] = 'Name is required';
            if(!_.isEmpty(errors))
                return errors;
        },
        url: function() {
            return restUri(this.isNew()?'/type':('/type/' + this.get('type_id')));
        }
    }
    );

var TypeCollection = RestCollection.extend(
    {
        model: Type,
        url: restUri('/type')
    }
    );

var Item = RestModel.extend(
    {
        defaults: {
            type_id: 0,
            maker_id: 0,
            maker_name: '',
            item_name: '',
            item_notes: '',
            item_year: null,
            item_cond: 0,
            item_cond_notes: ''
        },
        parse: function(json) {
            var result = RestModel.prototype.parse(json);
            if(result['item_year'] == '')
                result['item_year'] = null;
            return result;
        },
        idAttribute: 'item_id',
        validate: function() {
            var errors = {};
            if(this.get('name') == '')
                errors['name'] = 'Name is required';
            if(!_.isEmpty(errors))
                return errors;
        },
        url: function() {
            return restUri(this.isNew()?'/item':('/item/' + this.get('item_id')));
        }
    }
    );

var Attachment = RestModel.extend(
    {
        idAttribute: 'attachment_id',
        defaults: {
            attachment_title: '',
            attachment_orig_filename: '',
            attachment_upload_data: ''
        },
        url: function() {
            if(this.isNew())
                return 'attachment';
            else
                return 'attachment/' + this.get('attachment_id') + '/info';
        },
        urlFullsize: function() {
            return 'attachment/' + this.get('attachment_id') + '/image'
        },
        urlThumb: function() {
            return 'attachment/' + this.get('attachment_id') + '/image/150x150'
        },
        urlMedium: function() {
            return 'attachment/' + this.get('attachment_id') + '/image/400x300'
        }
    }
    );

var ItemImageCollection = RestCollection.extend(
    {
        initialize: function(models, options) {
            RestCollection.prototype.initialize.apply(this, arguments);
            this.item = options.item;
        },
        model: Attachment,
        url: function() { return restUri('/item/' + this.item.get('item_id') + '/image'); }
    }
    );

var ItemCollection = RestCollection.extend(
    {
        model: Item,
        url: restUri('/item')
    }
    );

var Maker = RestModel.extend(
    {
        defaults: {
            maker_name: '',
            item_count: 0
        },
        idAttribute: 'maker_id',
        validate: function() {
            var errors = {};
            if(this.get('maker_name') == '')
                errors['maker_name'] = 'Name is required';
            if(!_.isEmpty(errors))
                return errors;
        },
        url: function() {
            return restUri(this.isNew()?'/maker':('/maker/' + this.get('maker_id')));
        }
    }
    );

var MakerCollection = RestCollection.extend(
    {
        model: Maker,
        url: restUri('/maker')
    }
    );

var MakerItems = RestCollection.extend(
    {
        model: Item,
        initialize: function(items, options) {
            RestCollection.prototype.initialize.apply(this, arguments);
            this._maker = options.maker;
        },
        url: function() { return restUri('/maker/' + this._maker.get('maker_id') + '/item'); }
    }
    );

var TypeItems = RestCollection.extend(
    {
        model: Item,
        initialize: function(items, options) {
            RestCollection.prototype.initialize.apply(this, arguments);
            this._type = options.type;
        },
        url: function() { return restUri('/type/' + this._type.get('type_id') + '/item'); }
    }
    );

/*
 * Monostate options object - simple key/value settings for the whole application.
 */
var Options = RestModel.extend(
    {
        defaults: {
            collection_name: 'New Collection'
        },
        url: restUri('/option'),
        isNew: function() { return false; }
    }
    );
