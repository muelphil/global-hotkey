export type KeyCombination = {
    meta: boolean,
    shift: boolean,
    ctrl: boolean,
    alt: boolean,
    keyCode: string
}

export function addListener(keyCombination: KeyCombination, listener: () => void): number

export function removeListener(id: number): void

export function startListening(): void

export function stopListening(): void