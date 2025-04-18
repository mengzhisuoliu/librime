//
// Copyright RIME Developers
// Distributed under the BSD License
//
// 2011-4-6 Zou xu <zouivex@gmail.com>
//
#ifndef RIME_CONFIG_TYPES_H_
#define RIME_CONFIG_TYPES_H_

#include <type_traits>
#include <rime_api.h>
#include <rime/common.h>

namespace rime {

// config item base class
class ConfigItem {
 public:
  enum ValueType { kNull, kScalar, kList, kMap };

  ConfigItem() = default;  // null
  virtual ~ConfigItem() = default;

  ValueType type() const { return type_; }

  virtual bool empty() const { return type_ == kNull; }

 protected:
  ConfigItem(ValueType type) : type_(type) {}

  ValueType type_ = kNull;
};

class ConfigValue : public ConfigItem {
 public:
  ConfigValue() : ConfigItem(kScalar) {}
  RIME_DLL ConfigValue(bool value);
  RIME_DLL ConfigValue(int value);
  RIME_DLL ConfigValue(double value);
  RIME_DLL ConfigValue(const char* value);
  RIME_DLL ConfigValue(const string& value);

  // schalar value accessors
  bool GetBool(bool* value) const;
  RIME_DLL bool GetInt(int* value) const;
  bool GetDouble(double* value) const;
  RIME_DLL bool GetString(string* value) const;
  bool SetBool(bool value);
  bool SetInt(int value);
  bool SetDouble(double value);
  bool SetString(const char* value);
  bool SetString(const string& value);

  const string& str() const { return value_; }

  bool empty() const override { return value_.empty(); }

 protected:
  string value_;
};

class ConfigList : public ConfigItem {
 public:
  using Sequence = vector<of<ConfigItem>>;
  using Iterator = Sequence::iterator;

  ConfigList() : ConfigItem(kList) {}
  RIME_DLL an<ConfigItem> GetAt(size_t i) const;
  RIME_DLL an<ConfigValue> GetValueAt(size_t i) const;
  RIME_DLL bool SetAt(size_t i, an<ConfigItem> element);
  bool Insert(size_t i, an<ConfigItem> element);
  RIME_DLL bool Append(an<ConfigItem> element);
  bool Resize(size_t size);
  RIME_DLL bool Clear();
  RIME_DLL size_t size() const;

  Iterator begin();
  Iterator end();

  bool empty() const override { return seq_.empty(); }

 protected:
  Sequence seq_;
};

// limitation: map keys have to be strings, preferably alphanumeric
class ConfigMap : public ConfigItem {
 public:
  using Map = map<string, an<ConfigItem>>;
  using Iterator = Map::iterator;

  ConfigMap() : ConfigItem(kMap) {}
  RIME_DLL bool HasKey(const string& key) const;
  RIME_DLL an<ConfigItem> Get(const string& key) const;
  RIME_DLL an<ConfigValue> GetValue(const string& key) const;
  RIME_DLL bool Set(const string& key, an<ConfigItem> element);
  bool Clear();

  Iterator begin();
  Iterator end();

  bool empty() const override { return map_.empty(); }

 protected:
  Map map_;
};

namespace {

template <class T>
an<ConfigItem> AsConfigItem(const T& x, const std::false_type&) {
  return New<ConfigValue>(x);
};

template <class T>
an<ConfigItem> AsConfigItem(const T& x, const std::true_type&) {
  return x;
};

}  // namespace

class ConfigData;
class ConfigListEntryRef;
class ConfigMapEntryRef;

class ConfigItemRef {
 public:
  ConfigItemRef(ConfigData* data) : data_(data) {}
  virtual ~ConfigItemRef() = default;
  operator an<ConfigItem>() const { return GetItem(); }
  an<ConfigItem> operator*() const { return GetItem(); }
  template <class T>
  ConfigItemRef& operator=(const T& x) {
    SetItem(AsConfigItem(x, std::is_convertible<T, an<ConfigItem>>()));
    return *this;
  }
  ConfigListEntryRef operator[](size_t index);
  ConfigMapEntryRef operator[](const string& key);

  RIME_DLL bool IsNull() const;
  bool IsValue() const;
  RIME_DLL bool IsList() const;
  bool IsMap() const;

  RIME_DLL bool ToBool() const;
  RIME_DLL int ToInt() const;
  double ToDouble() const;
  RIME_DLL string ToString() const;

  RIME_DLL an<ConfigList> AsList();
  RIME_DLL an<ConfigMap> AsMap();
  RIME_DLL void Clear();

  // list
  RIME_DLL bool Append(an<ConfigItem> item);
  RIME_DLL size_t size() const;
  // map
  RIME_DLL bool HasKey(const string& key) const;

  RIME_DLL bool modified() const;
  RIME_DLL void set_modified();

 protected:
  virtual an<ConfigItem> GetItem() const = 0;
  virtual void SetItem(an<ConfigItem> item) = 0;

  ConfigData* data_;
};

class ConfigListEntryRef : public ConfigItemRef {
 public:
  ConfigListEntryRef(ConfigData* data, an<ConfigList> list, size_t index)
      : ConfigItemRef(data), list_(list), index_(index) {}
  using ConfigItemRef::operator=;

 protected:
  an<ConfigItem> GetItem() const { return list_->GetAt(index_); }
  void SetItem(an<ConfigItem> item) {
    list_->SetAt(index_, item);
    set_modified();
  }

 private:
  an<ConfigList> list_;
  size_t index_;
};

class ConfigMapEntryRef : public ConfigItemRef {
 public:
  ConfigMapEntryRef(ConfigData* data, an<ConfigMap> map, const string& key)
      : ConfigItemRef(data), map_(map), key_(key) {}
  using ConfigItemRef::operator=;

 protected:
  an<ConfigItem> GetItem() const { return map_->Get(key_); }
  void SetItem(an<ConfigItem> item) {
    map_->Set(key_, item);
    set_modified();
  }

 private:
  an<ConfigMap> map_;
  string key_;
};

inline ConfigListEntryRef ConfigItemRef::operator[](size_t index) {
  return ConfigListEntryRef(data_, AsList(), index);
}

inline ConfigMapEntryRef ConfigItemRef::operator[](const string& key) {
  return ConfigMapEntryRef(data_, AsMap(), key);
}

}  // namespace rime

#endif  // RIME_CONFIG_TYPES_H_
