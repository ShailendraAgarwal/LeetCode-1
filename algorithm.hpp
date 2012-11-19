#include <functional>
#include <vector>
#include <set>
#include <map>
#include <utility>
#include <iostream>

namespace jcui {
namespace algorithm {

// Return first location where comp(a, b) is not true.
template<class ForwardIterator1, class ForwardIterator2, class Compare>
ForwardIterator1 lower_bound(ForwardIterator1 begin1,
			     ForwardIterator1 end1,
			     ForwardIterator2 begin2,
			     ForwardIterator2 end2,
			     Compare comp) {
  int len = end1 - begin1;
  while (len > 0) {
    int half = len >> 1;
    ForwardIterator1 m1 = begin1 + half;
    ForwardIterator2 m2 = begin2 + half;
    if (comp(*m1, *m2)) {
      begin1 = m1 + 1;
      begin2 = m2 + 1;
      len -= (half + 1);
    } else {
      len = half;
    }
  }
  return begin1;
}

// Return min_x max(inc[x], dec[x]) where inc and dec are non-decreasing and non-increasing   
template<class T, class ForwardIterator1, class ForwardIterator2> 
T min_max(ForwardIterator1 inc_begin,
	  ForwardIterator1 inc_end,
	  ForwardIterator2 dec_begin,
	  ForwardIterator2 dec_end,
	  T default_value) {
  int len = inc_end - inc_begin;
  if (len == 0) return default_value;
  ForwardIterator1 it = lower_bound(inc_begin, inc_end, dec_begin, dec_end, std::less<T>());
  if (it == inc_begin) return *inc_begin;
  if (it == inc_end) return *(dec_begin + len - 1);
  return min(*it, *(dec_begin + (it - inc_begin - 1)));
}

template<class T>
T pow(const T& x, int k) {
  if (k <= 1) return x;
  T y = pow(x, k / 2);
  y = T::mul(y, y);
  if (k % 2) y = T::mul(y, x);
  return y;
}
  
template<class T>
class Mat {
public:
  Mat(int H, int W) {
    v.resize(H);
    for (int i = 0; i < H; ++i) v[i].resize(W);
  }
  int H() const { return v.size(); }
  int W() const { return v.size() ? v[0].size() : 0; }  
  void set(int i, int j, T val) { if (valid(i, j)) v[i][j] = val; }
  T get(int i, int j) const { return valid(i, j) ? v[i][j] : 0; }
  void normalize_by_row_sum() {
    std::vector<T> s = Mat<T>::row_sum(*this);
    for (int i = 0; i < H(); ++i) {
      for (int j = 0; j < W(); ++j) {
	v[i][j] /= s[j];
      }
    }
  }
  static std::vector<T> row_sum(const Mat& A) {
    std::vector<T> y(A.W(), 0);
    for (int j = 0; j < A.W(); ++j) {
      for (int i = 0; i < A.H(); ++i) {
	y[j] += A.v[i][j];
      }
    }
    return y;
  }
  static Mat mul(const Mat& x, const Mat& y) {
    Mat p(x.H(), y.W());
    for (int i = 0; i < x.H(); ++i) {
      for (int j = 0; j < y.W(); ++j) {
	T v = 0;
	for (int k = 0; k < x.W(); ++k) {
	  v += x.v[i][k] * y.v[k][j];
	}
	p.v[i][j] = v;
      }      
    }
    return p;
  }
  std::vector<T> operator*(const std::vector<T>& x) const {
    std::vector<T> y(H(), 0);
    for (int i = 0; i < H(); ++i) {
      for (int j = 0; j < W(); ++j) {
	y[i] += v[i][j] * x[j];
      }
    }
    return y;
  }
  inline bool valid(int i, int j) const { return i >= 0 && i < H() && j >= 0 && j < W(); }
  void print() const {
    for (int i = 0; i < H(); ++i) {
      for (int j = 0; j < W(); ++j) {
	std::cout << v[i][j] << " ";
      }
      std::cout << std::endl;
    }
  }
  std::vector<std::vector<T> > v;
};

template<class T>
class SparseMat {
public:
  typedef typename std::map<std::pair<int, int>, T>::const_iterator MapConstIter;
  typedef std::set<int>::iterator SetIter;

  SparseMat(int H, int W) {
    row_ids.resize(W);
    col_ids.resize(H);
  }
  int H() const { return col_ids.size(); }
  int W() const { return row_ids.size(); }
  void set(int i, int j, T val) {
    if (i < 0 || i >= H() || j < 0 || j >= W()) return;
    v[std::make_pair(i, j)] = val;
    rows.insert(i);
    cols.insert(j);
    row_ids[j].insert(i);
    col_ids[i].insert(j);
  }
  T get(int i, int j) const {
    MapConstIter it = v.find(std::make_pair(i, j));
    if (it == v.end()) return 0;
    return it->second;
  }
  void normalize_by_row_sum() {
    std::vector<T> s = SparseMat<T>::row_sum(*this);
    for (SetIter col = cols.begin(); col != cols.end(); ++col) {
      for (SetIter row = row_ids[*col].begin(); row != row_ids[*col].end(); ++row) {
	v[std::make_pair(*row, *col)] /= s[*col];
      }
    }
  }
  static std::vector<T> row_sum(const SparseMat& A) {
    std::vector<T> y(A.W(), 0);
    for (SetIter col = A.cols.begin(); col != A.cols.end(); ++col) {
      for (SetIter row = A.row_ids[*col].begin(); row != A.row_ids[*col].end(); ++row) {
	y[*col] += A.v.find(std::make_pair(*row, *col))->second;
      }
    }
    return y;
  }
  std::vector<T> operator*(const std::vector<T>& x) const {
    std::vector<T> y(H(), 0);
    for (SetIter r = rows.begin(); r != rows.end(); ++r) {
      for (SetIter k = col_ids[*r].begin(); k != col_ids[*r].end(); ++k) {
	y[*r] += v.find(std::make_pair(*r, *k))->second * x[*k];
      }
    }
    return y;
  }
  static SparseMat mul(const SparseMat& x, const SparseMat& y) {
    SparseMat p(x.H(), y.W());
    std::vector<int> intersect(x.W());
    typedef std::vector<int>::iterator VecIter;
    for (SetIter r = x.rows.begin(); r != x.rows.end(); ++r) {
      for (SetIter c = y.cols.begin(); c != y.cols.end(); ++c) {
	T v = 0;
	VecIter intersect_end = set_intersection(x.col_ids[*r].begin(),
						 x.col_ids[*r].end(),
						 y.row_ids[*c].begin(), 
						 y.row_ids[*c].end(), 
						 intersect.begin());
	for (VecIter k = intersect.begin(); k != intersect_end; ++k) {
	  v += x.v.find(std::make_pair(*r, *k))->second * y.v.find(std::make_pair(*k, *c))->second;
	}
	if (v != 0) p.set(*r, *c, v);
      }
    }
    return p;
  }
  void print() const {
    for (MapConstIter i = v.begin(); i != v.end(); ++i) {
      std::cout << "(" << i->first.first << ", " << i->first.second << ") = " << i->second << std::endl;
    }
  }
  std::map<std::pair<int, int>, T> v;
  std::set<int> rows, cols;
  std::vector<std::set<int> > row_ids, col_ids;
};

template<class T>
class RingBuffer {
public:
  RingBuffer(int N) : cur(0), size(0) { v.resize(N); } // N > 0
  void push_back(const T& val) {
    v[cur] = val;
    cur ++;
    if (cur >= v.size()) cur = 0;
    if (size < v.size()) size ++;
  }
  // Get value from last_write_position + offset
  T get(int offset, T default_val = 0) const {
    if (offset > 0 || offset <= -size) return default_val; // Cannot read future info, or info that got flushed
    int index = cur - 1 + offset;
    if (index < 0) index += size;
    return v[index];
  }     
  int N() const { return size; }
private:
  int cur;
  int size;
  std::vector<T> v;
};

} // namespace algorithm
} // namespace jcui
