import java.util.HashMap;


public final class Inversions {

	public static int count(int[] perm) {
		int[] aux = new int[perm.length];

		int hi = perm.length-1;
		int lo = 0;

		int numInversions = sortAndMerge(perm, lo, hi, aux);

		return numInversions;
	}

	private static int sortAndMerge( int[] a, int lo, int hi, int[] aux ) {
		if ( hi - lo == 0 ) return 0;

		int mid = lo + (hi - lo)/2;

		int lc = sortAndMerge(a, lo, mid, aux);
		int rc = sortAndMerge(a, mid+1, hi, aux);
		int r = mergeAndCount(a, lo, mid, mid+1, hi, aux);

		return r+lc+rc;

	}

	private static int mergeAndCount(int[] a, int llo, int lhi, int rlo, int rhi, int[] aux) {
		int c = 0;
		int i = llo;
		int j = rlo;

		// copy to aux[]
		for ( int k = llo; k <= rhi; ++k ) {
			aux[k] = a[k]; 
		}

		for ( int k = llo; k <= rhi; ++k ) {
			if (i > lhi) {
				a[k] = aux[j++];
			} else if (j > rhi) {
				a[k] = aux[i++];
			} else if (less(aux[j], aux[i])) {
				c += lhi - i + 1;
				a[k] = aux[j++];
			} else
				a[k] = aux[i++];
		}

		return c;
	}

	private static boolean less(int i, int j) {
		return i < j;
	}
	
	/**
	 * Get the number of inversions between two ranked lists
	 * @param first
	 * @param second
	 * @return
	 */
	private static int getDifference(int[] first, int[] second) {
		assert first.length == second.length;
		
		int N = first.length;
		HashMap<Integer, Integer> map = new HashMap<Integer, Integer>();
		
		for ( int i = 0; i < N; ++i ) {
			map.put(first[i], i);
		}
		
		int[] perm = new int[N];
		for ( int i = 0; i < N; ++i ) {
			perm[i] = map.get(second[i]);
		}
		
		return count(perm);
	}

	/**
	 * @param args
	 */
	public static void main(String[] args) {

		int[] perm = {1, 4, 5, 6, 7, 2, 3};

		System.out.println("The original array is: "+toString(perm));

		System.out.println("Number of inversions of the array is "+ count(perm));

		System.out.println("Sorted array is: "+toString(perm));
		
		int[] f = {1, 2, 3, 4, 7, 5, 6};
		int[] s = {1, 4, 5, 6, 7, 2, 3};
		
		System.out.println("Difference in ranks of f ["+toString(f)+"] and s ["+toString(s)+"] = "+getDifference(f, s));
	}

	private static String toString(int[] perm) {
		String s = "";
		for ( int i : perm )
			s += (i+" ");
		return s.trim();
	}

}

