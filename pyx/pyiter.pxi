

cdef Chain iters_to_c_chain(list ob):
    cdef IterWrapper wrapped
    cdef AnyIter it
    cdef Chain c_chain
    for wrapped in ob:
        it = wrapped.iter
        c_chain.push_back(it)
    return c_chain


cdef class TubeIter:
    cdef ToPyIter *output
    cdef list iters # reference kept here to enusure the iters live long enough
    cdef Chain chain;

    def __cinit__(self, IterWrapper output, list iters, list root_chain, *a):
        self.output = <ToPyIter*>output.iter.get()
        self.iters = iters
        self.chain = iters_to_c_chain(root_chain)


cdef class TubeSingleIter(TubeIter):

    def __next__(self):
        do_next(self.chain)
        cdef ToPyIter *out = self.output
        cdef PyObj obj = out.get(0).acquire()
        return <object>obj.give()


cdef class TubeMultiIter(TubeIter):
    cdef size_t num_slots

    def __cinit__(self, IterWrapper output, list iters, list root_chain, *a):
        self.num_slots, = a

    def __next__(self):
        do_next(self.chain)
        cdef ToPyIter *out = self.output
        return tuple(<object>out.get(i).acquire().give() for i in range(self.num_slots))


cdef class Chains:
    cdef public Tube output
    cdef public dict chains
    cdef public dict ordering
    cdef public set inputs

    def __cinit__(self, Tube output):
        self.output = output
        self.chains = {}
        self.ordering = {}
        self.inputs = set()
        self.add(output, (None, output), 0)

    def add(self, tube, parent, rank=0):
        self.chains.setdefault(parent, set())
        self.chains[parent].add(tube)
        self.ordering[tube] = max(rank, self.ordering.get(tube, -1))

        owned_inputs = set()
        for chain in tube._chains:
            for owned_tube in chain:
                self.add(owned_tube, (tube, chain), rank+1)
                owned_inputs.add(owned_tube)

        for tube_input in tube._inputs:
            if tube_input not in owned_inputs:
                self.add(tube_input, parent, rank+1)

        if not tube._inputs:
            self.inputs.add(tube)

    def ordered(self):
        chain_order = lambda i: max(self.ordering[t] for t in i[1])
        ordered_chains = sorted(self.chains.items(), key=chain_order, reverse=True)
        for key, tubes in ordered_chains:
            ordered_tubes = sorted(tubes, key=lambda t: self.ordering[t], reverse=True)
            yield key, ordered_tubes

    def verify(self):
        seen = set()
        for chain in self.chains.values():
            if seen & chain:
                raise ValueError("Tube in multiple chains")
            seen.update(chain)

    cdef make_chains_iters(self, modifiers=None):
        cdef Tube tube
        self.verify()
        if modifiers is None:
            modifiers = {}
        made_chains = {}
        made_iters = {}
        
        for key, tubes in self.ordered():
            this_chain = []
            for tube in tubes:
                args = []
                for calls_next in tube._chains:
                    next_key = (tube, calls_next)
                    args.append(made_chains[next_key])
                for tube_input in tube._inputs:
                    args.append(made_iters[tube_input])
                the_iter = tube._make_iter(args)
                if tube in modifiers:
                    modifiers[tube](the_iter)
                made_iters[tube] = the_iter
                this_chain.append(the_iter)
            made_chains[key] = this_chain

        cdef Tube output = self.output
        num_slots = len(output.dtype)

        return made_chains, made_iters
