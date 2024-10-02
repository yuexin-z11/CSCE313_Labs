#include <threading.h>

void t_init()
{
        // TODO
        getcontext(&contexts[0].context);
        contexts[0].state = VALID;
        current_context_idx = 0;
}

int32_t t_create(fptr foo, int32_t arg1, int32_t arg2)
{
        // TODO
        // find free slot in contexts[]
        for(volatile size_t i = 0; i < NUM_CTX; i++){
                if (contexts[i].state == INVALID){
                        // initialize
                        getcontext(&contexts[i].context);
                        contexts[i].context.uc_stack.ss_sp = malloc(STK_SZ);
                        contexts[i].context.uc_stack.ss_size = STK_SZ;
                        contexts[i].context.uc_stack.ss_flags = 0;
                        contexts[i].state = VALID;
                        contexts[i].context.uc_link = &contexts[current_context_idx].context;

                        makecontext(&contexts[i].context, (ctx_ptr)foo, 2, arg1, arg2);
                        return 0; // success
                }
        }

        return 1;

}

int32_t t_yield()
{
        // TODO
        // switch context to another thread if there is one; otherwise continue with the caller
        int32_t v_text = 0;
        int32_t tmp_ctx = current_context_idx;

        for (volatile int32_t i = 1; i < NUM_CTX; i++){
                int next = (current_context_idx + i) % NUM_CTX;
                if (contexts[next].state == VALID){
                        v_text++;
                        tmp_ctx = current_context_idx;
                        current_context_idx = (uint8_t)next;
                        swapcontext(&contexts[tmp_ctx].context, &contexts[next].context);

                        break;
                }
        }

        return v_text;
}

void t_finish()
{
        // TODO
        // marks current thread as finished and remove from scheduling
        contexts[current_context_idx].state = DONE;
        free(contexts[current_context_idx].context.uc_stack.ss_sp);
        t_yield();
}
