/*
 * Fooyin
 * Copyright © 2023, Luke Taylor <LukeT1@proton.me>
 *
 * Fooyin is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Fooyin is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Fooyin.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#pragma once

#include "helpers.h"

#include <vector>

namespace Fooyin {
template <class Item>
class TreeItem
{
public:
    explicit TreeItem(Item* parent = nullptr)
        : m_parent{parent}
    { }

    virtual ~TreeItem()
    {
        m_children.clear();
    }

    virtual bool hasChild(Item* child) const
    {
        return Utils::contains(m_children, child);
    }

    virtual std::vector<Item*> children() const
    {
        return m_children;
    }

    virtual void appendChild(Item* child)
    {
        m_children.emplace_back(child);
        child->m_parent = static_cast<Item*>(this);
    }

    virtual void insertChild(int row, Item* child)
    {
        row = std::min(row, static_cast<int>(m_children.size()));
        m_children.insert(m_children.begin() + row, child);
        child->m_parent = static_cast<Item*>(this);
    }

    virtual void moveChild(int oldRow, int newRow)
    {
        if(oldRow < newRow) {
            Utils::move(m_children, oldRow, newRow - 1);
        }
        else {
            Utils::move(m_children, oldRow, newRow);
        }
    }

    virtual bool removeChild(int index)
    {
        if(index < 0 || index >= childCount()) {
            return false;
        }
        m_children.erase(m_children.cbegin() + index);
        return true;
    }

    virtual void clearChildren()
    {
        m_children.clear();
    }

    virtual Item* child(int index) const
    {
        if(index < 0 || index >= childCount()) {
            return nullptr;
        }
        return m_children.at(index);
    }

    [[nodiscard]] virtual int childCount() const
    {
        return static_cast<int>(m_children.size());
    }

    [[nodiscard]] virtual int row() const
    {
        if(m_row < 0 && m_parent) {
            m_row = Utils::findIndex(m_parent->m_children, this);
        }
        return m_row;
    }

    [[nodiscard]] virtual Item* parent() const
    {
        return m_parent;
    }

    virtual void resetRow()
    {
        m_row = -1;
    }

    virtual void resetChildren()
    {
        for(Item* child : m_children) {
            if(child) {
                child->resetChildren();
                child->resetRow();
            }
        }
    }

    template <typename SortFunc>
    void sortChildren(SortFunc&& func)
    {
        for(auto* child : m_children) {
            child->sortChildren(std::forward<SortFunc>(func));
            child->resetRow();
        }

        std::ranges::sort(m_children, std::forward<SortFunc>(func));
    }

    void sortChildren()
    {
        sortChildren([](const Item* a, const Item* b) {
            if(!a || !b) {
                return false;
            }
            return *a < *b;
        });
    }

private:
    friend Item;

    Item* m_parent;                // Not owned
    std::vector<Item*> m_children; // Not owned
    mutable int m_row{-1};
};
} // namespace Fooyin
