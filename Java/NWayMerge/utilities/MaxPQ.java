package utilities;

import java.util.Comparator;
import java.util.Iterator;
import java.util.NoSuchElementException;

public class MaxPQ<Key> implements Iterable<Key> {
	private Key[] _pq;                    // store items at indices 1 to N
	private int _N;                       // number of items on priority queue
	private Comparator<Key> _comparator;  // optional Comparator

	/**
	 * 
	 * @param capacity
	 */
	@SuppressWarnings("unchecked")
	public MaxPQ(int capacity) {
		_pq = (Key[]) new Object[capacity + 1];
		_N = 0;
	}

	/**
	 * 
	 */
	public MaxPQ() { this(1); }

	/**
	 * 
	 * @param initCapacity
	 * @param comparator
	 */
	@SuppressWarnings("unchecked")
	public MaxPQ(int initCapacity, Comparator<Key> comparator) {
		this._comparator = comparator;
		_pq = (Key[]) new Object[initCapacity + 1];
		_N = 0;
	}

	/**
	 * 
	 * @param comparator
	 */
	public MaxPQ(Comparator<Key> comparator) { this(1, comparator); }

	/**
	 * 
	 * @param keys
	 */
	@SuppressWarnings("unchecked")
	public MaxPQ(Key[] keys) {
		_N = keys.length;
		_pq = (Key[]) new Object[keys.length + 1]; 
		for (int i = 0; i < _N; i++)
			_pq[i+1] = keys[i];
		for (int k = _N/2; k >= 1; k--)
			sink(k);
		assert isMaxHeap();
	}

	/**
	 * 
	 * @return
	 */
	public boolean isEmpty() {
		return _N == 0;
	}

	/**
	 * 
	 * @return
	 */
	public int size() {
		return _N;
	}

	/**
	 * 
	 * @return
	 */
	public Key max() {
		if (isEmpty()) throw new NoSuchElementException("Priority queue underflow");
		return _pq[1];
	}

	/**
	 * 
	 * @param capacity
	 */
	@SuppressWarnings("unchecked")
	private void resize(int capacity) {
		assert capacity > _N;
		Key[] temp = (Key[]) new Object[capacity];
		for (int i = 1; i <= _N; i++) temp[i] = _pq[i];
		_pq = temp;
	}


	/**
	 * 
	 * @param x
	 */
	public void insert(Key x) {

		// double size of array if necessary
		if (_N >= _pq.length - 1) resize(2 * _pq.length);

		// add x, and percolate it up to maintain heap invariant
		_pq[++_N] = x;
		swim(_N);
		assert isMaxHeap();
	}

	/**
	 * 
	 * @return
	 */
	public Key delMax() {
		if (isEmpty()) throw new NoSuchElementException("Priority queue underflow");
		Key max = _pq[1];
		exch(1, _N--);
		sink(1);
		_pq[_N+1] = null;     // to avoid loiterig and help with garbage collection
		if ((_N > 0) && (_N == (_pq.length - 1) / 4)) resize(_pq.length / 2);
		assert isMaxHeap();
		return max;
	}


	///////////////////////////////// Helper functions to restore the heap invariant
	/////////////////////////////////////////////////////////////////////////////////
	/**
	 * 
	 */
	private void swim(int k) {
		while (k > 1 && less(k/2, k)) {
			exch(k, k/2);
			k = k/2;
		}
	}
	
	/**
	 * 
	 * @param k
	 */
	private void sink(int k) {
		while (2*k <= _N) {
			int j = 2*k;
			if (j < _N && less(j, j+1)) j++;
			if (!less(k, j)) break;
			exch(k, j);
			k = j;
		}
	}

	///////////////////////////////////////// Helper functions for compares and swaps
	/////////////////////////////////////////////////////////////////////////////////
	@SuppressWarnings("unchecked")
	private boolean less(int i, int j) {
		if (_comparator == null) {
			return ((Comparable<Key>) _pq[i]).compareTo(_pq[j]) < 0;
		}
		else {
			return _comparator.compare(_pq[i], _pq[j]) < 0;
		}
	}

	/**
	 * 
	 * @param i
	 * @param j
	 */
	private void exch(int i, int j) {
		Key swap = _pq[i];
		_pq[i] = _pq[j];
		_pq[j] = swap;
	}

	/**
	 * 
	 * @return
	 */
	private boolean isMaxHeap() {
		return isMaxHeap(1);
	}

	/**
	 * 
	 * @param k
	 * @return
	 */
	private boolean isMaxHeap(int k) {
		if (k > _N) return true;
		int left = 2*k, right = 2*k + 1;
		if (left  <= _N && less(k, left))  return false;
		if (right <= _N && less(k, right)) return false;
		return isMaxHeap(left) && isMaxHeap(right);
	}


	/**
	 * 
	 */
	public Iterator<Key> iterator() { return new HeapIterator(); }

	/**
	 * 
	 * @author data
	 *
	 */
	private class HeapIterator implements Iterator<Key> {

		// create a new pq
		private MaxPQ<Key> _copy;

		// add all items to copy of heap
		// takes linear time since already in heap order so no keys move
		public HeapIterator() {
			if (_comparator == null) _copy = new MaxPQ<Key>(size());
			else                    _copy = new MaxPQ<Key>(size(), _comparator);
			for (int i = 1; i <= _N; i++)
				_copy.insert(_pq[i]);
		}

		public boolean hasNext()  { return !_copy.isEmpty();                     }
		public void remove()      { throw new UnsupportedOperationException();  }

		public Key next() {
			if (!hasNext()) throw new NoSuchElementException();
			return _copy.delMax();
		}

		
	}
	
	@Override
	public String toString() {
		StringBuilder sb = new StringBuilder("[");
		for ( Key key : this )
			sb.append(key+", ");
		String s = sb.toString().trim()+"]";
		return s;
	}

}
