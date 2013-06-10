package utilities;

/**
 * 
 * @author data
 * 
 * This Pair class is derived from StanfordCoreNLP library 
 *
 * @param <T1>
 * @param <T2>
 */
public class Pair <T1,T2> implements Comparable<Pair<T1,T2>> {

	public T1 first;
	public T2 second;

	public Pair() {
		first = null; second = null; // default initialization
	}

	public Pair(T1 first, T2 second) {
		this.first = first;
		this.second = second;
	}

	public T1 first() {
		return first;
	}

	public T2 second() {
		return second;
	}

	public void setFirst(T1 o) {
		first = o;
	}

	public void setSecond(T2 o) {
		second = o;
	}

	@Override
	public String toString() {
		return "(" + first + "," + second + ")";
	}

	@Override
	@SuppressWarnings({ "rawtypes" })
	public boolean equals(Object o) {
		if (o instanceof Pair) {
			Pair p = (Pair) o;
			return (first == null ? p.first == null : first.equals(p.first)) && (second == null ? p.second == null : second.equals(p.second));
		} else {
			return false;
		}
	}

	@Override
	public int hashCode() {
		return (((first == null) ? 0 : first.hashCode()) << 16) ^ ((second == null) ? 0 : second.hashCode());
	}


	public static <P, Q> Pair<P, Q> makePair(P x, Q y) {
		return new Pair<P, Q>(x, y);
	}

	@SuppressWarnings("unchecked")
	public int compareTo(Pair<T1,T2> another) {
		int comp = ((Comparable<T1>) first()).compareTo(another.first());
		if (comp != 0) {
			return comp;
		} else {
			return ((Comparable<T2>) second()).compareTo(another.second());
		}
	}

}
