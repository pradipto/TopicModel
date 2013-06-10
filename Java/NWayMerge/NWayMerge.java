

import java.util.ArrayList;
import java.util.List;

import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;


import utilities.MinPQ;
import utilities.Pair;

@SuppressWarnings("rawtypes")
public class NWayMerge<Key extends Comparable<Key>> {
	
	/**
	 * Mapper class
	 * 
	 * @author data
	 *
	 */
	@SuppressWarnings("hiding")
	private class Mapper<Key extends Comparable<Key>> implements Runnable {
		
		private Key[] _gArr = null;
		private int _begin = -1;
		private int _end = -1;

		private Combiner _c = null;

		public Mapper(Combiner c) {
			_c = c;
		}
		
		public void initialize(Key[] a, int begin, int end) {
			_gArr = a;
			_begin = begin;
			_end = end;
		}
		
		@SuppressWarnings("unchecked")
		@Override
		public void run() {
			_c.combine(_gArr, _begin, _end);
		}

		@Override
		public String toString() {
			StringBuilder s = new StringBuilder("Mapper [_begin=" + _begin + ", _end=" + _end + " (");
			for ( int i = _begin; i <= _end; ++i )
				s.append(_gArr[i]+",");
			s.append(")]");
			return s.toString();
		}
		
	}
	
	/**
	 * Combiner class
	 * 
	 * @author data
	 *
	 */
	@SuppressWarnings("hiding")
	private class Combiner<Key extends Comparable<Key>> {

		private MinPQ<Key> _pq = null;
		
		public void combine(Key[] gArr, int begin, int end) {
			_pq = new MinPQ<Key>();
			for ( int i = begin; i <= end; ++i )
				_pq.insert(gArr[i]);
		}
		
		/**
		 * Ideally the priority queues can be sub-classed from a priority queue base class
		 * @return
		 */
		public MinPQ<Key> getPriorityQueue() {
			return _pq;
		}
		
		public Key getNextItem() {
			if ( !_pq.isEmpty() ) {
				return _pq.delMin();
			} else 
				return null;
		}
		
	}
	
	/**
	 * Reducer class
	 * 
	 * @author data
	 *
	 */
	@SuppressWarnings("hiding")
	private class Reducer<Key extends Comparable<Key>> {
		
		private Combiner[] _combiners = null;
		private int _T;
		private int i = -1;
		private ArrayList<Key> _outputList;
		
		public Reducer(int numMappers) {
			_T = numMappers;
			_combiners = new Combiner[_T];
			_outputList = new ArrayList<Key>();
		}
		
		public void addCombiner(Combiner c) {
			_combiners[++i] = c;
		}
		
		@SuppressWarnings("unchecked")
		public void reduce() {
			MinPQ<Pair<Key, Integer>> pqIndexed = new MinPQ<Pair<Key, Integer>>();
			for ( int t = 0; t < _T; ++t ) {
				pqIndexed.insert(new Pair<Key, Integer>((Key)_combiners[t].getPriorityQueue().delMin(), t));
			}
			
			while ( !pqIndexed.isEmpty() ) {
				Pair<Key, Integer> p = pqIndexed.delMin();
				Key minValue = p.first();
				Integer mapperIndex = p.second();
				
				_outputList.add(minValue);
				
				pull(pqIndexed, mapperIndex);
			}
		}

		@SuppressWarnings("unchecked")
		private void pull(MinPQ<Pair<Key, Integer>> pqIndexed, Integer mapperIndex) {
			Key value = (Key) _combiners[mapperIndex].getNextItem();
			// if there are no items in the requested combiner then value is null
			if ( value != null ) {
				pqIndexed.insert(new Pair<Key, Integer>(value, mapperIndex));
			}
		}
		
		public List<Key> getOutputList() {
			return _outputList;
		}
	}
	
	////////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////// N Way Merge ////////////
	
	////// Executor framework
	private int _T;				// number of threads
	ExecutorService _executor;	// needed for thread pool
	
	private Key[] _gArr;
	private Mapper[] _mappers;
	private Combiner[] _combiners;
	private Reducer _reducer;
	
	/**
	 * 
	 * @param arr
	 * @param numThreads
	 */
	public NWayMerge(Key[] arr, int numThreads) {
		_gArr = arr;
		_T = numThreads;
		if ( _T > arr.length ) _T = Math.max(1, arr.length/10);
		_executor = Executors.newFixedThreadPool(_T);
		_mappers = new Mapper[_T];
		_combiners = new Combiner[_T];
		_reducer = new Reducer(_T);
	}
	
	/**
	 * 
	 */
	@SuppressWarnings("unchecked")
	public void map() {
		int V = _gArr.length;
		int chunkSize = V/_T;
		for ( int i = 0, t = 0; i < V && t < _T; i += chunkSize, ++t ) {
			Combiner c = new Combiner();
			_combiners[t] = c;
			
			Mapper m = new Mapper(c);
			
			int begin = i;
			int end = -1;
			
			if (i + 2*chunkSize - 1 >= V ) {
				end = V-1;
			} else {
				end = i+chunkSize-1;
			}
			m.initialize(_gArr, begin, end);
			
			//System.out.println(t+") "+i+":"+(i+chunkSize-1)+":"+(i+2*chunkSize-1)+"/"+V+" -> begin "+begin+", end "+end);
			_mappers[t] = m;
			
		}

		// run the mappers
		for ( int t = 0; t < _T; ++t ) {
			_executor.execute(_mappers[t]);
		}
		// This will make the executor accept no new threads
		// and finish all existing threads in the queue
		_executor.shutdown();
		// Wait until all threads are finish
		while (!_executor.isTerminated()) { }
		System.out.println("Finished all mappers");
	    
	}
	
	/**
	 * 
	 */
	public void reduce() {
		
		for ( int t = 0; t < _T; ++t ) 
			_reducer.addCombiner(_combiners[t]);
		
		_reducer.reduce();
		
		System.out.println("Printing final sorted list");
		int i = 1;
		for ( Object d : _reducer.getOutputList() ) 
			System.out.println((i++)+") "+(Double)d+" ");
	}
	
		
	
	/**
	 * Test client
	 * @param args
	 */
	public static void main(String[] args) {
		
		int V = 105;
		int nThreads = 10;
		Double[] arr = new Double[V];
		for ( int i = 0; i < V; ++i )
			arr[i] = Math.random()*10;
		
		NWayMerge mapreduceApp = new NWayMerge<Double>(arr, nThreads);
		mapreduceApp.map();
		mapreduceApp.reduce();
		
	}

	@SuppressWarnings("unused")
	private static void printArray(Double[] arr) {
		int N = arr.length;
		for  ( int i = 0; i < N; ++i ) {
			System.out.print(arr[i]+" ");
			if ( (i+1) % 10 == 0 ) System.out.println();
		}
		System.out.println();
		
	}

}

