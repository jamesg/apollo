// REST responses consist of the actual response as the 'data' key inside a
// JSON object.  This allows the protocol to be extended more easily at a later
// date if required.  In the case of array responses, returning a raw array is
// also a potential security risk.

var RestCollection = Backbone.Collection.extend(
        {
            parse: function(response) {
                return response.data;
            }
        }
        );

var RestModel = Backbone.Model.extend(
        {
            parse: function(response) {
                if(_.has(response, 'data'))
                    return response.data;
                else
                    return response;
            }
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
            return this.isNew()?'/type':('/type/' + this.get('type_id'));
        }
    }
    );

var TypeCollection = RestCollection.extend(
    {
        model: Type,
        url: '/type'
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
            item_year: 1970,
            item_cond: 0,
            item_cond_notes: ''
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
            return this.isNew()?'/item':('/item/' + this.get('item_id'));
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
                return '/attachment';
            else
                return '/attachment/' + this.get('attachment_id') + '/info';
        },
        urlFullsize: function() {
            return '/attachment/' + this.get('attachment_id') + '/image'
        },
        urlThumb: function() {
            return '/attachment/' + this.get('attachment_id') + '/image/150x150'
        },
        urlMedium: function() {
            return '/attachment/' + this.get('attachment_id') + '/image/400x300'
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
        url: function() { return '/item/' + this.item.get('item_id') + '/image'; }
    }
    );

var ItemCollection = RestCollection.extend(
    {
        model: Item,
        url: '/item'
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
            return this.isNew()?'/maker':('/maker/' + this.get('maker_id'));
        }
    }
    );

var MakerCollection = RestCollection.extend(
    {
        model: Maker,
        url: '/maker'
    }
    );

var MakerItems = RestCollection.extend(
    {
        model: Item,
        initialize: function(items, options) {
            RestCollection.prototype.initialize.apply(this, arguments);
            this._maker = options.maker;
        },
        url: function() { return '/maker/' + this._maker.get('maker_id') + '/item'; }
    }
    );

