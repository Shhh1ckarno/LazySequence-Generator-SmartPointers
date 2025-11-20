#pragma once

#include <cstddef>
#include <utility>
#include <new>
#include <typeinfo>
#include <cassert>
#include <iostream>
#include <functional>
#include <stdexcept>
#include <optional>
#include <string>

#include "ArraySequence.h"
#include "DynamicArray.h"
#include "SmartPointer.h"
#include "Cardinal.h"
#include "Generator.h"
#include "MutableArraySequence.h"
#include "ImmutableArraySequence.h"

template <class T> class LazySequenceBase;
template <class T> class CoreLazySequence;
template <class T> class AppendedLazySequence;
template <class T> class PrependedLazySequence;
template <class T> class InsertedAtLazySequence;
template <class T, class R> class MapLazySequence;
template <class T> class WhereLazySequence;
template <class T, class U> class ZipLazySequence;

template <class T>
class LazySequenceBase {
public:
    virtual ~LazySequenceBase() {}

    virtual T Get(size_t index) = 0;
    virtual Cardinal GetLength() const = 0;
    virtual size_t GetMaterializedCount() const = 0;

    virtual SharedPtr< LazySequenceBase<T> > Append(const T& item) = 0;
    virtual SharedPtr< LazySequenceBase<T> > Prepend(const T& item) = 0;
    virtual SharedPtr< LazySequenceBase<T> > InsertAt(const T& item, size_t index) = 0;

    virtual SharedPtr< LazySequenceBase<T> > Clone() const = 0;

    template <class R>
    SharedPtr< LazySequenceBase<R> > Map(R (*f)(T)) {
        return make_shared< MapLazySequence<T,R> >( this->Clone(), f );
    }

    SharedPtr< LazySequenceBase<T> > Where(bool (*pred)(T)) {
        return make_shared< WhereLazySequence<T> >( this->Clone(), pred );
    }

    template<class U>
    SharedPtr< LazySequenceBase< std::pair<T,U> > > Zip(const SharedPtr< LazySequenceBase<U> >& other) {
        return make_shared< ZipLazySequence<T,U> >( this->Clone(), other );
    }

};

template <class T>
class CoreLazySequence : public LazySequenceBase<T> {
public:
    CoreLazySequence() {
        materialised = make_shared< MutableArraySequence<T> >();
        rule = nullptr;
        wrapperRule = nullptr;
    }

    CoreLazySequence(T* items, int count) {
        materialised = make_shared< MutableArraySequence<T> >();
        for (int i = 0; i < count; ++i) materialised->Append(items[i]);
        rule = nullptr;
        wrapperRule = nullptr;
    }

    CoreLazySequence(Sequence<T>* seq) {
        ArraySequence<T>* arr = dynamic_cast< ArraySequence<T>* >(seq);
        if (!arr) throw std::runtime_error("CoreLazySequence(Sequence*): only ArraySequence derived supported");
        materialised = make_shared< MutableArraySequence<T> >();
        size_t n = arr->GetLength();
        for (size_t i = 0; i < n; ++i) materialised->Append(arr->Get(i));
        rule = nullptr;
        wrapperRule = nullptr;
    }

    CoreLazySequence(T (*ruleFunc)(Sequence<T>*), Sequence<T>* seedSeq) {
        materialised = make_shared< MutableArraySequence<T> >();
        if (seedSeq) {
            ArraySequence<T>* arr = dynamic_cast< ArraySequence<T>* >(seedSeq);
            if (!arr) throw std::runtime_error("CoreLazySequence(rule, seq): only ArraySequence derived supported");
            size_t n = arr->GetLength();
            for (size_t i = 0; i < n; ++i) materialised->Append(arr->Get(i));
        }
        rule = ruleFunc;
        wrapperRule = nullptr;
    }

    CoreLazySequence(std::function<T(Sequence<T>*)> ruleFunc, Sequence<T>* seedSeq) {
        materialised = make_shared< MutableArraySequence<T> >();
        if (seedSeq) {
            ArraySequence<T>* arr = dynamic_cast< ArraySequence<T>* >(seedSeq);
            if (!arr) throw std::runtime_error("CoreLazySequence(std::function, seq): only ArraySequence derived supported");
            size_t n = arr->GetLength();
            for (size_t i = 0; i < n; ++i) materialised->Append(arr->Get(i));
        }
        wrapperRule = ruleFunc;
        rule = nullptr;
    }

    CoreLazySequence(const CoreLazySequence<T>& other) {
        materialised = make_shared< MutableArraySequence<T> >();
        size_t n = other.materialised->GetLength();
        for (size_t i = 0; i < n; ++i) materialised->Append(other.materialised->Get(i));
        rule = other.rule;
        wrapperRule = other.wrapperRule;
        children = other.children;
    }

    SharedPtr< LazySequenceBase<T> > Clone() const override {
        return make_shared< CoreLazySequence<T> >(*this);
    }

    void SetGenerator(T (*ruleFunc)(Sequence<T>*)) { rule = ruleFunc; }
    void SetGenerator(std::function<T(Sequence<T>*)> ruleFunc) { wrapperRule = ruleFunc; }

    T GetFirst() {
        if (GetMaterializedCount() == 0 && !HasAnyGenerator()) throw std::out_of_range("IndexOutOfRange: empty");
        return Get(0);
    }

    T GetLast() {
        Cardinal len = GetLength();
        if (len.IsOmega()) throw std::runtime_error("GetLast on infinite sequence");
        size_t n = len.GetValue();
        if (n == 0) throw std::out_of_range("IndexOutOfRange: empty");
        return Get(n - 1);
    }

T Get(size_t index) override {
    size_t n = materialised->GetLength();
    if (index < n) return materialised->Get(index);

    // Проходим через дочерние последовательности
    size_t offset = n;
    for (int i = 0; i < children.GetSize(); ++i) {
        Cardinal childLen = children.Get(i)->GetLength();
        if (!childLen.IsOmega() && index < offset + childLen.GetValue())
            return children.Get(i)->Get(index - offset);
        if (childLen.IsOmega()) {
            // бесконечная последовательность
            return children.Get(i)->Get(index - offset);
        }
        offset += childLen.GetValue();
    }

    throw std::out_of_range("CoreLazySequence::Get: index beyond materialised and children");
}
    SharedPtr< LazySequenceBase<T> > GetSubsequence(int startIndex, int endIndex) {
        if (startIndex < 0 || endIndex < startIndex) throw std::out_of_range("GetSubsequence: invalid indices");
        for (int i = startIndex; i <= endIndex; ++i) Get((size_t)i);
        int len = endIndex - startIndex + 1;
        T* buffer = new T[len];
        for (int i = 0; i < len; ++i) buffer[i] = Get((size_t)(startIndex + i));
        SharedPtr< CoreLazySequence<T> > out = make_shared< CoreLazySequence<T> >(buffer, len);
        delete [] buffer;
        return out;
    }

    Cardinal GetLength() const override {
        if (rule || wrapperRule) return Cardinal::Omega();
        if (children.GetSize() > 0) {
            for (int i = 0; i < children.GetSize(); ++i) {
                Cardinal c = children.Get(i)->GetLength();
                if (c.IsOmega()) return Cardinal::Omega();
            }
            size_t sum = materialised->GetLength();
            for (int i = 0; i < children.GetSize(); ++i) sum += children.Get(i)->GetLength().GetValue();
            return Cardinal(sum);
        }
        return Cardinal(materialised->GetLength());
    }

    size_t GetMaterializedCount() const override { return materialised->GetLength(); }

    SharedPtr< LazySequenceBase<T> > Append(const T& item) override {
        return make_shared< AppendedLazySequence<T> >( this->Clone(), item );
    }
    SharedPtr< LazySequenceBase<T> > Prepend(const T& item) override {
        return make_shared< PrependedLazySequence<T> >( this->Clone(), item );
    }
    SharedPtr< LazySequenceBase<T> > InsertAt(const T& item, size_t index) override {
        return make_shared< InsertedAtLazySequence<T> >( this->Clone(), item, index );
    }

    SharedPtr< ArraySequence<T> > GetMaterialisedArray() { return materialised; }
    void AddChild(const SharedPtr< LazySequenceBase<T> >& child) { children.Append(child); }

bool HasAnyGenerator() const {
    if (rule || wrapperRule) return true;
    for (int i = 0; i < children.GetSize(); ++i) {
        if (children.Get(i)->GetLength().IsOmega()) return true;
    }
    return false;
}

    T (*GetRawRule())(Sequence<T>*) { return rule; }
    std::function<T(Sequence<T>*)> GetWrapperRule() const { return wrapperRule; }

private:
    SharedPtr< ArraySequence<T> > materialised;
    T (*rule)(Sequence<T>*) = nullptr;
    std::function<T(Sequence<T>*)> wrapperRule = nullptr;
    DynamicArray< SharedPtr< LazySequenceBase<T> > > children;
};


template <class T>
class AppendedLazySequence : public LazySequenceBase<T> {
public:
    AppendedLazySequence(const SharedPtr< LazySequenceBase<T> >& base_, const T& value) : base(base_), item(value) {}
    AppendedLazySequence(const AppendedLazySequence& other) : base(other.base), item(other.item) {}

    SharedPtr< LazySequenceBase<T> > Clone() const override {
        return ::make_shared< AppendedLazySequence<T> >(*this);
    }

    T Get(size_t index) override {
        Cardinal len = base->GetLength();
        if (!len.IsOmega()) {
            size_t bl = len.GetValue();
            if (index < bl) return base->Get(index);
            if (index == bl) return item;
            throw std::out_of_range("IndexOutOfRange in Appended");
        } else {
            return base->Get(index);
        }
    }

    Cardinal GetLength() const override {
        Cardinal bl = base->GetLength();
        if (bl.IsOmega()) return Cardinal::Omega();
        return Cardinal(bl.GetValue() + 1);
    }

    size_t GetMaterializedCount() const override { return base->GetMaterializedCount() + 1; }

    SharedPtr< LazySequenceBase<T> > Append(const T& v) override { return ::make_shared< AppendedLazySequence<T> >( this->Clone(), v ); }
    SharedPtr< LazySequenceBase<T> > Prepend(const T& v) override { return ::make_shared< PrependedLazySequence<T> >( this->Clone(), v ); }
    SharedPtr< LazySequenceBase<T> > InsertAt(const T& v, size_t idx) override { return ::make_shared< InsertedAtLazySequence<T> >( this->Clone(), v, idx ); }

private:
    SharedPtr< LazySequenceBase<T> > base;
    T item;
};


template <class T>
class PrependedLazySequence : public LazySequenceBase<T> {
public:
    PrependedLazySequence(const SharedPtr< LazySequenceBase<T> >& base_, const T& value) : base(base_), item(value) {}
    PrependedLazySequence(const PrependedLazySequence& other) : base(other.base), item(other.item) {}

    SharedPtr< LazySequenceBase<T> > Clone() const override {
        return ::make_shared< PrependedLazySequence<T> >(*this);
    }

    T Get(size_t index) override {
        if (index == 0) return item;
        return base->Get(index - 1);
    }

    Cardinal GetLength() const override {
        Cardinal bl = base->GetLength();
        if (bl.IsOmega()) return Cardinal::Omega();
        return Cardinal(bl.GetValue() + 1);
    }

    size_t GetMaterializedCount() const override { return base->GetMaterializedCount() + 1; }

    SharedPtr< LazySequenceBase<T> > Append(const T& v) override { return ::make_shared< AppendedLazySequence<T> >( this->Clone(), v ); }
    SharedPtr< LazySequenceBase<T> > Prepend(const T& v) override { return ::make_shared< PrependedLazySequence<T> >( this->Clone(), v ); }
    SharedPtr< LazySequenceBase<T> > InsertAt(const T& v, size_t idx) override { return ::make_shared< InsertedAtLazySequence<T> >( this->Clone(), v, idx ); }

private:
    SharedPtr< LazySequenceBase<T> > base;
    T item;
};


template <class T>
class InsertedAtLazySequence : public LazySequenceBase<T> {
public:
    InsertedAtLazySequence(const SharedPtr< LazySequenceBase<T> >& base_, const T& value, size_t index_) : base(base_), item(value), idx(index_) {}
    InsertedAtLazySequence(const InsertedAtLazySequence& other) : base(other.base), item(other.item), idx(other.idx) {}

    SharedPtr< LazySequenceBase<T> > Clone() const override {
        return ::make_shared< InsertedAtLazySequence<T> >(*this);
    }

    T Get(size_t index) override {
        if (index == idx) return item;
        if (index < idx) return base->Get(index);
        return base->Get(index - 1);
    }

    Cardinal GetLength() const override {
        Cardinal bl = base->GetLength();
        if (bl.IsOmega()) return Cardinal::Omega();
        return Cardinal(bl.GetValue() + 1);
    }

    size_t GetMaterializedCount() const override { return base->GetMaterializedCount() + 1; }

    SharedPtr< LazySequenceBase<T> > Append(const T& v) override { return ::make_shared< AppendedLazySequence<T> >( this->Clone(), v ); }
    SharedPtr< LazySequenceBase<T> > Prepend(const T& v) override { return ::make_shared< PrependedLazySequence<T> >( this->Clone(), v ); }
    SharedPtr< LazySequenceBase<T> > InsertAt(const T& v, size_t index) override { return ::make_shared< InsertedAtLazySequence<T> >( this->Clone(), v, index ); }

private:
    SharedPtr< LazySequenceBase<T> > base;
    T item;
    size_t idx;
};


template <class T, class R>
class MapLazySequence : public LazySequenceBase<R> {
public:
    MapLazySequence(const SharedPtr< LazySequenceBase<T> >& base_, R (*f)(T)) : base(base_), func(f) {}
    MapLazySequence(const MapLazySequence& other) : base(other.base), func(other.func), cache(other.cache) {}

    SharedPtr< LazySequenceBase<R> > Clone() const override {
        return make_shared< MapLazySequence<T,R> >(*this);
    }

    R Get(size_t index) override {
        if ((size_t)cache.GetSize() > index) return cache.Get((int)index);
        T v = base->Get(index);
        R r = func(v);
        cache.Append(r);
        return r;
    }

    Cardinal GetLength() const override { return base->GetLength(); }
    size_t GetMaterializedCount() const override { return (size_t)cache.GetSize(); }

    SharedPtr< LazySequenceBase<R> > Append(const R& v) override { return make_shared< AppendedLazySequence<R> >( this->Clone(), v ); }
    SharedPtr< LazySequenceBase<R> > Prepend(const R& v) override { return make_shared< PrependedLazySequence<R> >( this->Clone(), v ); }
    SharedPtr< LazySequenceBase<R> > InsertAt(const R& v, size_t idx) override { return make_shared< InsertedAtLazySequence<R> >( this->Clone(), v, idx ); }

private:
    SharedPtr< LazySequenceBase<T> > base;
    R (*func)(T);
    DynamicArray<R> cache;
};


template <class T>
class WhereLazySequence : public LazySequenceBase<T> {
public:
    WhereLazySequence(const SharedPtr< LazySequenceBase<T> >& base_, bool (*p)(T)) : base(base_), pred(p) {}
    WhereLazySequence(const WhereLazySequence& other) : base(other.base), pred(other.pred), matches(other.matches) {}

    SharedPtr< LazySequenceBase<T> > Clone() const override {
        return make_shared< WhereLazySequence<T> >(*this);
    }

    T Get(size_t index) override {
        ensureFound(index);
        size_t baseIndex = matches.Get((int)index);
        return base->Get(baseIndex);
    }

    Cardinal GetLength() const override {
        Cardinal bl = base->GetLength();
        if (bl.IsOmega()) return Cardinal::Omega();
        size_t n = bl.GetValue();
        size_t cnt = 0;
        for (size_t i = 0; i < n; ++i) if (pred(base->Get(i))) ++cnt;
        return Cardinal(cnt);
    }

    size_t GetMaterializedCount() const override { return (size_t)matches.GetSize(); }

    SharedPtr< LazySequenceBase<T> > Append(const T& v) override { return make_shared< AppendedLazySequence<T> >( this->Clone(), v ); }
    SharedPtr< LazySequenceBase<T> > Prepend(const T& v) override { return make_shared< PrependedLazySequence<T> >( this->Clone(), v ); }
    SharedPtr< LazySequenceBase<T> > InsertAt(const T& v, size_t idx) override { return make_shared< InsertedAtLazySequence<T> >( this->Clone(), v, idx ); }

private:
    void ensureFound(size_t idx) {
        if ((size_t)matches.GetSize() > idx) return;
        size_t start = (matches.GetSize() == 0) ? 0 : (matches.Get(matches.GetSize()-1) + 1);
        Cardinal bl = base->GetLength();
        size_t cur = start;
        if (bl.IsOmega()) {
            while ((size_t)matches.GetSize() <= idx) {
                T v = base->Get(cur);
                if (pred(v)) matches.Append(cur);
                ++cur;
            }
        } else {
            size_t limit = bl.GetValue();
            while ((size_t)matches.GetSize() <= idx && cur < limit) {
                T v = base->Get(cur);
                if (pred(v)) matches.Append(cur);
                ++cur;
            }
            if ((size_t)matches.GetSize() <= idx) throw std::out_of_range("Where: no more elements");
        }
    }

    SharedPtr< LazySequenceBase<T> > base;
    bool (*pred)(T);
    DynamicArray<size_t> matches;
};


template <class T, class U>
class ZipLazySequence : public LazySequenceBase< std::pair<T,U> > {
public:
    ZipLazySequence(const SharedPtr< LazySequenceBase<T> >& a_, const SharedPtr< LazySequenceBase<U> >& b_) : a(a_), b(b_) {}
    ZipLazySequence(const ZipLazySequence& other) : a(other.a), b(other.b) {}

    SharedPtr< LazySequenceBase< std::pair<T,U> > > Clone() const override {
        return make_shared< ZipLazySequence<T,U> >(*this);
    }

    std::pair<T,U> Get(size_t index) override { return std::make_pair(a->Get(index), b->Get(index)); }

    Cardinal GetLength() const override {
        Cardinal la = a->GetLength();
        Cardinal lb = b->GetLength();
        if (la.IsOmega() || lb.IsOmega()) return Cardinal::Omega();
        size_t na = la.GetValue(), nb = lb.GetValue();
        return Cardinal( na < nb ? na : nb );
    }

    size_t GetMaterializedCount() const override {
        size_t ma = a->GetMaterializedCount();
        size_t mb = b->GetMaterializedCount();
        return ma < mb ? ma : mb;
    }

    SharedPtr< LazySequenceBase< std::pair<T,U> > > Append(const std::pair<T,U>& v) override {
        return ::make_shared< AppendedLazySequence< std::pair<T,U> > >( this->Clone(), v );
    }
    SharedPtr< LazySequenceBase< std::pair<T,U> > > Prepend(const std::pair<T,U>& v) override {
        return ::make_shared< PrependedLazySequence< std::pair<T,U> > >( this->Clone(), v );
    }
    SharedPtr< LazySequenceBase< std::pair<T,U> > > InsertAt(const std::pair<T,U>& v, size_t idx) override {
        return ::make_shared< InsertedAtLazySequence< std::pair<T,U> > >( this->Clone(), v, idx );
    }

private:
    SharedPtr< LazySequenceBase<T> > a;
    SharedPtr< LazySequenceBase<U> > b;
};


template <class T>
SharedPtr< LazySequenceBase<T> > Concat(const SharedPtr< LazySequenceBase<T> >& a,
                                        const SharedPtr< LazySequenceBase<T> >& b)
{
    if (!a) return b;
    if (!b) return a;

    Cardinal la = a->GetLength();
    Cardinal lb = b->GetLength();


    if (!la.IsOmega() && !lb.IsOmega()) {
        size_t na = la.GetValue();
        size_t nb = lb.GetValue();

        T* buf = new T[na + nb];
        for (size_t i = 0; i < na; ++i)
            buf[i] = a->Get(i);
        for (size_t j = 0; j < nb; ++j)
            buf[na + j] = b->Get(j);

        SharedPtr< LazySequenceBase<T> > out =
            ::make_shared< CoreLazySequence<T> >(buf, (int)(na + nb));
        delete [] buf;
        return out;
    }


    if (la.IsOmega()) {
        return a;
    }


    size_t na = la.GetValue();
    if (na == 0) {

        return b;
    }


    CoreLazySequence<T>* coreB = dynamic_cast< CoreLazySequence<T>* >(b.get());
    T (*rawRuleB)(Sequence<T>*) = nullptr;
    std::function<T(Sequence<T>*)> wrapperRuleB;

    if (coreB) {
        rawRuleB = coreB->GetRawRule();
        wrapperRuleB = coreB->GetWrapperRule();
    }

    if (!rawRuleB && !wrapperRuleB) {
        T* buf = new T[na];
        for (size_t i = 0; i < na; ++i)
            buf[i] = a->Get(i);

        auto out = ::make_shared< CoreLazySequence<T> >(buf, (int)na);
        delete [] buf;
        out->AddChild(b);
        return out;
    }

    SharedPtr< ArraySequence<T> > bSeed = coreB->GetMaterialisedArray();
    size_t nbSeed = bSeed ? bSeed->GetLength() : 0;

    T* buf = new T[na + nbSeed];


    for (size_t i = 0; i < na; ++i)
        buf[i] = a->Get(i);


    for (size_t j = 0; j < nbSeed; ++j)
        buf[na + j] = bSeed->Get(j);

    auto out = ::make_shared< CoreLazySequence<T> >(buf, (int)(na + nbSeed));
    delete [] buf;

    if (rawRuleB) {
        out->SetGenerator(rawRuleB);
    } else {
        out->SetGenerator(wrapperRuleB);
    }

    return out;
}


template <class T, class R>
R Reduce(const SharedPtr< LazySequenceBase<T> >& seq, R (*f)(R, T), R init) {
    Cardinal len = seq->GetLength();
    if (len.IsOmega()) throw std::runtime_error("Reduce on infinite sequence");
    size_t n = len.GetValue();
    R acc = init;
    for (size_t i = 0; i < n; ++i) acc = f(acc, seq->Get(i));
    return acc;
}

template <class T>
class LazySequence {
public:
    LazySequence() { root = make_shared< CoreLazySequence<T> >(); generator.reset(nullptr); }
    LazySequence(T* items, int count) { root = make_shared< CoreLazySequence<T> >(items, count); generator.reset(nullptr); }
    LazySequence(Sequence<T>* seq) { root = make_shared< CoreLazySequence<T> >(seq); generator.reset(nullptr); }
    LazySequence(T (*ruleFunc)(Sequence<T>*), Sequence<T>* seedSeq) {
        auto core = make_shared< CoreLazySequence<T> >(ruleFunc, seedSeq);
        root = core;
        generator = make_unique< Generator<T> >( core->GetMaterialisedArray(), ruleFunc );
    }
    LazySequence(std::function<T(Sequence<T>*)> ruleFunc, Sequence<T>* seedSeq) {
        auto core = make_shared< CoreLazySequence<T> >(ruleFunc, seedSeq);
        root = core;
        generator = make_unique< Generator<T> >( core->GetMaterialisedArray(), ruleFunc );
    }
    LazySequence(const LazySequence<T>& other) {
        root = other.root;
        if (other.generator) generator = make_unique< Generator<T> >(*other.generator);
        else generator.reset(nullptr);
    }

    void SetGenerator(T (*ruleFunc)(Sequence<T>*)) {
        CoreLazySequence<T>* core = dynamic_cast< CoreLazySequence<T>* >(root.get());
        if (!core) throw std::runtime_error("SetGenerator: root is not core");
        core->SetGenerator(ruleFunc);
        generator = make_unique< Generator<T> >( core->GetMaterialisedArray(), ruleFunc );
    }
    void SetGenerator(std::function<T(Sequence<T>*)> ruleFunc) {
        CoreLazySequence<T>* core = dynamic_cast< CoreLazySequence<T>* >(root.get());
        if (!core) throw std::runtime_error("SetGenerator: root is not core");
        core->SetGenerator(ruleFunc);
        generator = make_unique< Generator<T> >( core->GetMaterialisedArray(), ruleFunc );
    }

    T Get(size_t index) {
        // если индекс уже материализован — просто берём из root
        if (index < root->GetMaterializedCount()) {
            return root->Get(index);
        }

        // дальше нужны новые элементы -> нужен генератор
        if (!generator) {
            throw std::out_of_range(
                "LazySequence::Get: index is not materialized and no generator is attached"
            );
        }

        // генерируем, пока не появится нужный индекс
        while (root->GetMaterializedCount() <= index) {
            generator->GetNext();
        }
        return root->Get(index);
    }

    T GetFirst() {
        if (root->GetMaterializedCount() == 0 && !HasGenerator()) {
            throw std::out_of_range("LazySequence::GetFirst: sequence is empty");
        }
        return Get(0);
    }

    T GetLast() {
        Cardinal len = GetLength();
        if (len.IsOmega()) {
            throw std::runtime_error("LazySequence::GetLast: sequence is infinite");
        }
        size_t n = len.GetValue();
        if (n == 0) {
            throw std::out_of_range("LazySequence::GetLast: sequence is empty");
        }
        return Get(n - 1);
    }


    SharedPtr< LazySequenceBase<T> > GetRoot() const { return root; }
    Cardinal GetLength() const { return root->GetLength(); }
    size_t GetMaterializedCount() const { return root->GetMaterializedCount(); }

    void AppendValue(const T& v) { root = root->Append(v); }
    void PrependValue(const T& v) { root = root->Prepend(v); }
    void InsertAtValue(const T& v, size_t idx) { root = root->InsertAt(v, idx); }

// вставьте в класс LazySequence<T>
void ConcatWith(const SharedPtr< LazySequenceBase<T> >& other) {
        // сначала строим новый корень на уровне LazySequenceBase<T>
        SharedPtr< LazySequenceBase<T> > newRoot = Concat(root, other);

        // по умолчанию генератора нет
        generator.reset(nullptr);

      
        CoreLazySequence<T>* newCore = dynamic_cast< CoreLazySequence<T>* >(newRoot.get());
        if (newCore) {
            T (*rawRule)(Sequence<T>*) = newCore->GetRawRule();
            std::function<T(Sequence<T>*)> wrapperRule = newCore->GetWrapperRule();

            if (rawRule) {
                generator = ::make_unique< Generator<T> >(
                    newCore->GetMaterialisedArray(), rawRule
                );
            } else if (wrapperRule) {
                generator = ::make_unique< Generator<T> >(
                    newCore->GetMaterialisedArray(), wrapperRule
                );
            }
        }


        root = newRoot;
    }

    template <class R>
SharedPtr< LazySequenceBase<R> > Map(R (*f)(T)) {
    return root->template Map<R>(f);
}
    SharedPtr< LazySequenceBase<T> > Where(bool (*pred)(T)) { return root->Where(pred); }

    bool HasGenerator() const {
        CoreLazySequence<T>* core = dynamic_cast< CoreLazySequence<T>* >(root.get());
        if (!core) return false;
        return core->HasAnyGenerator();
    }

private:
    SharedPtr< LazySequenceBase<T> > root;
    UniquePtr< Generator<T> > generator;
};
